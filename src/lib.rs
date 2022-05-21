use std::os::unix::net::{UnixStream, UnixListener};

pub static SOCKET_PATH: &'static str = "/tmp/os.sock";

pub struct Config {
    pub filename: String,
    pub size: u32,
}

impl Config {
    pub fn new(args: &[String]) -> Result<Config, &str> {
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
