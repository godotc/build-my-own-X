use std::{
    fs::File,
    io::{self, Read, Write},
    num::ParseIntError,
    path::Path,
};

use nom::types::CompleteStr;

use crate::{assembler::program_parser::program, vm::VM};

pub struct REPL {
    command_buffer: Vec<String>,
    vm: VM,
}

impl REPL {
    pub fn new() -> REPL {
        REPL {
            command_buffer: vec![],
            vm: VM::new(),
        }
    }

    pub fn run(&mut self) -> ! {
        println!("Weclcome to the vm loop!");

        let stdin = io::stdin();
        loop {
            let mut buffer = String::new();

            print!("it> ");
            io::stdout().flush().expect("Unable to flush stdout");

            stdin
                .read_line(&mut buffer)
                .expect("Unable to read line from user");

            let buffer = buffer.trim();
            self.command_buffer.push(buffer.to_string());

            match buffer {
                ".quit" => {
                    println!("Farewell!");
                    std::process::exit(0);
                }
                ".load_file" => {
                    print!("Enter the path to the file you want to load: ");
                    io::stdout().flush().expect("Unable to flush stdout");
                    let mut tmp = String::new();
                    stdin
                        .read_line(&mut tmp)
                        .expect("Unable  to read line from user");
                    let tmp = tmp.trim();
                    let filename = Path::new(&tmp);
                    let mut f = File::open(filename).expect("Unable to open file ");
                    let mut content = String::new();
                    f.read_to_string(&mut content)
                        .expect("Error on reading this file");

                    let program = match program(CompleteStr(&content)) {
                        Ok((_remainder, program)) => program,
                        Err(e) => {
                            println!("Unable to parse input: {:?}", e);
                            continue;
                        }
                    };
                    // TODO & FIXME
                    // self.vm.program.append(&mut program.to_bytes());
                }
                ".history" => {
                    for cmd in &self.command_buffer {
                        println!("{}", cmd);
                    }
                }
                ".program" => {
                    println!("Listing instructions currently in VM's program vector:");
                    for instruction in &self.vm.program {
                        println!("{}", instruction);
                    }
                    println!("End of Program Listing");
                }
                ".registers" => {
                    println!("Listing registers currently in VM's register vector:");
                    println!("{:#?}", self.vm.registers);
                    println!("End of Register Listing");
                }
                _ => {
                    let program = match program(nom::types::CompleteStr(buffer)) {
                        Ok((_, program)) => program,
                        Err(_) => {
                            println!("Unable to  parse input");
                            continue;
                        }
                    };
                    // TODO & FIXME
                    // self.vm.program.append(&mut program.to_bytes());
                    // self.vm.program.append(&mut program.to_bytes());
                    self.vm.run_once();
                }
            }
        }
    }

    #[doc = r"accept hexdecimal string withoud start with `0x`"]
    /**
     * @return vec<u8>
     * examle a load command： 00 01 03 E8 // LOAD $1  0x03e8 or 0xe803?
     */
    ///
    #[allow(dead_code)]
    fn parse_hex(&mut self, i: &str) -> Result<Vec<u8>, ParseIntError> {
        let split = i.split(" ").collect::<Vec<&str>>();
        let mut results = vec![];

        for hex_str in split {
            let byte = u8::from_str_radix(&hex_str, 16); // 0x
            match byte {
                Ok(res) => {
                    results.push(res);
                }
                Err(err) => return Err(err),
            }
        }

        Ok(results)
    }
}
