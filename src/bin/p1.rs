use std::{
    env,
    fs::{self, File},
    path::Path,
    process, str,
    sync::mpsc,
    thread,
};

const MAX_THREADS: usize = 10;

struct Config {
    filename: String,
    size: u32,
}

impl Config {
    fn new(args: &[String]) -> Result<Config, &str> {
        if args.len() < 3 {
            return Err("Not enough arguments");
        }
        let filename = args[2].clone();
        let size = match args[1].parse() {
            Ok(x) => x,
            Err(_x) => return Err("Size should be integer"),
        };
        Ok(Config { filename, size })
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let config = Config::new(&args).unwrap_or_else(|err| {
        println!("Problem parsing arguments: {}", err);
        println!("Use: cargo run <size> <filename>");
        process::exit(1);
    });

    let path = Path::new(&config.filename);
    let mut numbers = Vec::new();
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
        let mut th_end = match th_start + offset as usize > content.len() {
            true => content.len(),
            false => th_start + offset as usize,
        };

        if th_end != content.len() {
            while content[th_end] != b' ' {
                th_end += 1;
            }
        }

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

    for msg in rx {
        for num in msg {
            numbers.push(num);
        }
    }

    println!("Loaded {} numbers into memory", config.size);

    // for handle in threads {
    //     handle.join().unwrap();
    // }
}
