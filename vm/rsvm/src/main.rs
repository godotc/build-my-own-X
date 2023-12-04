use std::{fs::File, io::Read, path::Path};

use clap::{load_yaml, App};
use log::info;
use rsvm::{assembler, repl, vm};

fn main() {
    env_logger::init();
    info!("Starting logging!");
    let yaml = load_yaml!("../cfg/cli.yml");
    let mathches = App::from_yaml(yaml).get_matches();
    let target_file = mathches.value_of("INPUT_FILE");
    match target_file {
        Some(filename) => {
            let program = read_file(filename);
            let mut asm = assembler::Assembler::new();
            let mut vm = vm::VM::new();

            let program = asm.assemble(&program);
            match program {
                Ok(p) => {
                    vm.add_bytes(p);
                    vm.run();
                    std::process::exit(0);
                }
                Err(_) => {}
            }
        }
        None => {
            start_repl();
        }
    }
}

fn start_repl() {
    let mut repl = repl::REPL::new();
    repl.run();
}
fn read_file(tmp: &str) -> String {
    let filename = Path::new(&tmp);
    match File::open(&filename) {
        Ok(mut file) => {
            let mut contents = String::new();
            match file.read_to_string(&mut contents) {
                Ok(_) => contents,
                Err(e) => {
                    println!("Error on read file: {:?}", e);
                    std::process::exit(1);
                }
            }
        }
        Err(e) => {
            println!("File not found: {:?}", e);
            std::process::exit(1);
        }
    }
}
