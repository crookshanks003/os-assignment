use std::{env, process};

use nix::unistd::*;

mod p1;
mod p2;

fn main() {
    let args: Vec<String> = env::args().collect();

    let config = os2::Config::new(&args).unwrap_or_else(|err| {
        println!("Problem parsing arguments: {}", err);
        println!("Use: cargo run <size> <filename>");
        process::exit(1);
    });

    match unsafe { fork() } {
        Ok(ForkResult::Parent { child:_ }) => {
            p2::main();
        }
        Ok(ForkResult::Child) => {
            p1::main(config);
        }
        Err(_) => {
            println!("Failed to fork");
        }
    }
}
