use std::{sync::mpsc, thread};

const MAX_THREADS: u32 = 10;

fn main() {
    let numbers: Vec<u32> = vec![1, 2, 3, 4, 5, 6];
    let size = 6;
    let mut sum: u32 = 0;

    let (tx, rx) = mpsc::channel();
    let rem = size%MAX_THREADS;
    let num_per_thread = size / MAX_THREADS;
    let mut index: usize = 0;

    for i in 0..MAX_THREADS {

        let th_tx = tx.clone();
        let num_this_thread = num_per_thread + (i < rem) as u32;
        let end_index = index+num_this_thread as usize;
        let th_numbers = numbers[index..end_index].to_vec();
        
        index = end_index;

        thread::spawn(move || {
            let loc_sum: u32 = th_numbers.iter().sum();
             
            th_tx.send(loc_sum).unwrap();
        });
    }

    drop(tx);

    for num in rx {
        sum += num;
    }

    println!("Sum: {}", sum);
}
