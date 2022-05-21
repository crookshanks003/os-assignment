use std::{fs, io::Read, os::unix::net::UnixListener, path::Path, process, sync::mpsc, thread};

use os2::SOCKET_PATH;

pub fn main() {
    let mut numbers: Vec<Vec<u32>> = Vec::new();
    let mut sum: u64 = 0;

    let (tx, rx) = mpsc::channel();

    let socket = Path::new(SOCKET_PATH);

    fs::remove_file(socket).unwrap_or_else(|_| {});

    let stream = match UnixListener::bind(&socket) {
        Ok(x) => {
            println!("Process 2 listening for numbers");
            x
        }
        Err(err) => {
            println!("Failed to bind to socket: {}", err);
            process::exit(1);
        }
    };

    match stream.accept() {
        Ok((mut listener, _addr)) => {
            let mut buf = String::new();
            listener.read_to_string(&mut buf).unwrap();
            numbers = serde_json::from_str(&buf).unwrap();
        }
        Err(_) => {
            println!("Could not read from the stream");
        }
    }

    for nums in numbers {
        let th_tx = tx.clone();

        thread::spawn(move || {
            let loc_sum: u32 = nums.iter().sum();

            th_tx.send(loc_sum).unwrap();
        });
    }

    drop(tx);

    for num in rx {
        sum += num as u64;
    }

    println!("Sum: {}", sum);
}
