pub mod command_parser;

use std::{
    fs::File,
    io::{self, Read, Write},
    num::ParseIntError,
    path::Path,
};

use crate::{
    assembler::{program_parser::program, Assembler},
    scheduler::Scheduler,
    vm::VM,
};

use self::command_parser::CommandParser;

#[derive(Debug, Clone)]
pub struct REPL {
    command_buffer: Vec<String>,
    vm: VM,
    asm: Assembler,
    scheduler: Scheduler,
}

impl Default for REPL {
    fn default() -> Self {
        Self::new()
    }
}

impl REPL {
    pub fn new() -> REPL {
        REPL {
            vm: VM::new(),
            command_buffer: vec![],
            asm: Assembler::new(),
            scheduler: Scheduler::new(),
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

            let history_copy = buffer.clone();
            self.command_buffer.push(history_copy);

            if buffer.starts_with("!") {
                self.execute_command(&buffer);
            } else {
                let program = match program(nom::types::CompleteStr(&buffer)) {
                    Ok((_, program)) => program,
                    Err(_) => {
                        println!("Unable to  parse input");
                        continue;
                    }
                };
                self.vm
                    .program
                    .append(&mut program.to_bytes(&self.asm.symbols));
                self.vm.run_once();
            }
        }
    }

    fn execute_command(&mut self, input: &str) {
        let args = CommandParser::tokenize(input);
        match args[0] {
            "!quit" => self.quit(&args[1..]),
            "!history" => self.history(&args[1..]),
            "!program" => self.program(&args[1..]),
            "!registers" => self.registers(&args[1..]),
            "!clear_program" => self.clear_program(&args[1..]),
            "!clear_registers" => self.clear_registers(&args[1..]),
            "!symbols" => self.symbols(&args[1..]),
            "!load_file" => self.load_file(&args[1..]),
            "!spawn" => self.spawn(&args[1..]),
            _ => println!("Invalid Command!"),
        }
    }

    fn quit(&mut self, args: &[&str]) {
        println!("Farewell!");
        std::process::exit(0);
    }

    fn history(&mut self, args: &[&str]) {
        for cmd in &self.command_buffer {
            println!("{}", cmd);
        }
    }

    fn program(&mut self, args: &[&str]) {
        println!("Listing instructions currently in VM's program vector:");
        for instruction in &self.vm.program {
            println!("{}", instruction);
        }
        println!("End of Program Listing");
    }

    fn registers(&mut self, args: &[&str]) {
        println!("Listing registers currently in VM's register vector:");
        println!("{:#?}", self.vm.registers);
        println!("End of Register Listing");
    }

    fn clear_program(&mut self, args: &[&str]) {
        println!("Removing all bytes from VM's program vector...");
        self.vm.program.truncate(0);
        println!("Done!");
    }

    fn clear_registers(&mut self, args: &[&str]) {
        println!("Clear all registers...");
        self.vm.registers = self.vm.registers.map(|_| 0);
        println!("Done!");
    }

    fn symbols(&mut self, args: &[&str]) {
        println!("Listing symbols table:");
        println!("{:#?}", self.asm.symbols);
        println!("End of Symbols Listing");
    }

    fn load_file(&mut self, args: &[&str]) {
        match self.get_data_from_load() {
            Some(content) => match self.asm.assemble(&content) {
                Ok(mut assembled_program) => {
                    println!("Sending assembled program to VM");
                    self.vm.program.append(&mut assembled_program);
                    println!("{:#?}", self.vm.program);
                    self.vm.run();
                }
                Err(errors) => {
                    for err in errors {
                        println!("Unable to parse input: {}", err);
                    }
                    return;
                }
            },
            None => return,
        }
    }

    fn spawn(&mut self, args: &[&str]) {
        let contents = self.get_data_from_load();
        if contents.is_none() {
            return;
        }
        match self.asm.assemble(&contents.unwrap()) {
            Ok(mut assembled_program) => {
                println!("Sending assembled program to VM");
                self.vm.program.append(&mut assembled_program);
                println!("{:#?}", self.vm.program);
                self.scheduler.get_thread(self.vm.clone());
            }
            Err(errors) => {
                for err in errors {
                    println!("Unable to parse input: {}", err);
                }
                return;
            }
        }
    }

    #[doc = r"accept hexdecimal string withoud start with `0x`"]
    /**
     * @return vec<u8>
     * examle a load command： 00 01 03 E8 // LOAD $1  0x03e8 or 0xe803?
     */
    ///
    fn parse_hex(&mut self, i: &str) -> Result<Vec<u8>, ParseIntError> {
        let split = i.split(' ').collect::<Vec<&str>>();
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

    fn get_data_from_load(&mut self) -> Option<String> {
        let stdin = io::stdin();
        print!("Enter the path to the file you want to load: ");
        io::stdout().flush().expect("Unable to flush stdout");
        let mut tmp = String::new();

        stdin
            .read_line(&mut tmp)
            .expect("Unable  to read line from user");
        println!("Attemping to load progream from file...");

        let tmp = tmp.trim();
        let filename = Path::new(&tmp);

        let mut f = match File::open(filename) {
            Ok(f) => f,
            Err(e) => {
                println!("Unable to open file: {:?}", e);
                return None;
            }
        };

        let mut content = String::new();
        match f.read_to_string(&mut content) {
            Ok(_num_read) => Some(content),
            Err(e) => {
                println!("Error on reading this file: {:?}", e);
                None
            }
        }
    }
}
