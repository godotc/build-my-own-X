use std::sync::Arc;
use std::{fs::File, io::Read, path::Path};

use clap::{load_yaml, App};
use log::info;
use vm::vm::VM;
use vm::{assembler, remote, repl};

fn main() {
    env_logger::init();
    info!("Starting logging!");
    let yaml = load_yaml!("cli.yml");
    let matches = App::from_yaml(yaml).get_matches();

    let num_threads = match matches.value_of("THREADS") {
        Some(num) => match num.parse::<usize>() {
            Ok(v) => v,
            Err(_e) => {
                let default = num_cpus::get();
                println!(
                    "Invalid argmument for number of threads: {}. using default dtected: {}.",
                    num, default
                );
                default
            }
        },
        None => num_cpus::get(),
    };

    if matches.is_present("add-ssh-key") {
        println!("User tried to add SSH key!");
        std::process::exit(0);
    }
    if matches.is_present("ENABLE_REMOTE_ACCESS") {
        let port = matches.value_of("LISTEN_PORT").unwrap_or("2223");
        let host = matches.value_of("LISTEN_HOST").unwrap_or("127.0.0.1");
        start_remote_server(host.to_string(), port.to_string());
    }

    if matches.is_present("ENABLE_SSH") {
        println!("User wants to enable SSH!");
        if matches.is_present("SSH_PORT") {
            let port = match matches.value_of("ssh_port") {
                Some(p) => match p.parse::<u32>() {
                    Ok(port) => port,
                    Err(_) => {
                        println!("Parse failed of input {}, use default...", p);
                        2223
                    }
                },
                None => 2223,
            };
            println!("They'd like to use port {:#?}", port);
            start_ssh_server(port)
        }
    }

    let target_file = matches.value_of("INPUT_FILE");

    match target_file {
        Some(filename) => {
            let program = read_file(filename);
            let mut asm = assembler::Assembler::new();
            let mut vm = VM::new();
            vm.logical_cores = num_threads;

            let program = asm.assemble(&program);
            match program {
                Ok(p) => {
                    vm.add_bytes(p);
                    let events = vm.run();
                    println!("VM Events...");
                    println!("-----------------------------------------");
                    for ev in &events {
                        println!("{:#?}", ev);
                    }
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

fn start_ssh_server(port: u32) {
    let _t = std::thread::spawn(move || {
        println!("TODO...");
        unimplemented!();
        // let mut config = thrussh::server::Config::default();
        // config.connection_timeout = Some(std::time::Duration::from_secs(500));
        // config.auth_rejection_time = std::time::Duration::from_secs(3);
        // let config = Arc::new(config);
        // let sh = vm::ssh::Server {};
        // thrussh::server::run(config, format!("127.0.0.1:{}", port).as_str(), sh);
    });
}

fn start_remote_server(host: String, port: String) {
    println!("Listening on {}:{}", host, port);
    let _t = std::thread::spawn(move || {
        let mut sh = remote::server::Server::new(host, port);
        sh.listen();
    });
}
