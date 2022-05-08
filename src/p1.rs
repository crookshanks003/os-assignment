use std::io::Write;
use std::os::unix::net::UnixStream;
use std::{
    fs::{self, File},
    path::Path,
    process,
    sync::mpsc,
    thread,
};

const MAX_THREADS: usize = 10;

use os2::{Config, SOCKET_PATH};

pub fn main(config: Config) {
    let path = Path::new(&config.filename);
    let mut numbers: Vec<Vec<u32>> = Vec::new();
    let f = File::open(path).unwrap_or_else(|err| {
        println!("Error reading file: {}", err);
        process::exit(1);
    });

    let content = fs::read(path).unwrap();

    let file_size = f.metadata().unwrap().len();
    let offset = file_size / MAX_THREADS as u64;

    let (tx, rx) = mpsc::channel();

    let mut handles: Vec<thread::JoinHandle<()>> = Vec::new();

    let mut th_start: usize = 0;

    for _i in 0..MAX_THREADS {
        let th_tx = tx.clone();

        let th_end = get_th_end(offset, th_start, &content);
        let th_nums = content[th_start..th_end].to_vec();
        th_start = th_end;

        let th_handle = thread::spawn(move || {
            let s = String::from_utf8(th_nums).unwrap();

            let buf: Vec<u32> = s.split_whitespace().map(|st| st.parse().unwrap()).collect();

            th_tx.send(buf).unwrap();
        });

        handles.push(th_handle);
    }
    drop(tx);

    let socket = Path::new(SOCKET_PATH);

    let mut stream = UnixStream::connect(&socket).unwrap_or_else(|err| {
        println!("Failed to connect to socket: {}", err);
        process::exit(1);
    });

    for msg in rx {
        numbers.push(msg);
    }

    let buf = serde_json::to_vec(&numbers).unwrap();
    stream.write_all(&buf).unwrap();

    println!("Loaded {} numbers into memory", config.size);
}

fn get_th_end(offset: u64, th_start: usize, content: &[u8]) -> usize {
    let mut th_end = match th_start + offset as usize > content.len() {
        true => content.len(),
        false => th_start + offset as usize,
    };

    if th_end != content.len() {
        while content[th_end] != b' ' {
            th_end += 1;
        }
    }
    th_end
}
