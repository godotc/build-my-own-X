pub mod command_parser;

use std::{
    fmt::format,
    fs::File,
    io::{self, Read, Write},
    num::ParseIntError,
    path::Path,
    sync::mpsc,
    sync::mpsc::{Receiver, Sender},
};

use crate::{
    assembler::{program_parser::program, Assembler},
    scheduler::Scheduler,
    vm::VM,
};

use self::command_parser::CommandParser;

pub static REMOTE_BANNER: &'static str = "Welcome to Irdium! Let's be productive!";
pub static PROMPT: &'static str = "it> ";
pub static COMMAND_PREFIX: &'static str = "!";

#[derive(Debug)]
pub struct REPL {
    command_buffer: Vec<String>,
    vm: VM,
    asm: Assembler,
    scheduler: Scheduler,

    pub tx_pipe: Option<Box<Sender<String>>>,
    pub rx_pipe: Option<Box<Receiver<String>>>,
}

impl Default for REPL {
    fn default() -> Self {
        Self::new()
    }
}

impl REPL {
    pub fn new() -> REPL {
        let (tx, rx): (Sender<_>, Receiver<_>) = mpsc::channel();
        REPL {
            vm: VM::new(),
            command_buffer: vec![],
            asm: Assembler::new(),
            scheduler: Scheduler::new(),
            tx_pipe: Some(Box::new(tx)),
            rx_pipe: Some(Box::new(rx)),
        }
    }

    pub fn run(&mut self) -> ! {
        println!("{}", REMOTE_BANNER.to_string());

        let stdin = io::stdin();
        loop {
            let mut buffer = String::new();

            print!("{}", PROMPT.to_string());
            io::stdout().flush().expect("Unable to flush stdout");

            stdin
                .read_line(&mut buffer)
                .expect("Unable to read line from user");

            let history_copy = buffer.clone();
            self.command_buffer.push(history_copy);

            if buffer.starts_with(COMMAND_PREFIX) {
                self.execute_command(&buffer);
            } else {
                let program = match program(nom::types::CompleteStr(&buffer)) {
                    Ok((_, program)) => program,
                    Err(_) => {
                        self.send_message(format!("Unable to  parse input"));
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

    pub fn run_single(&mut self, buffer: &str) -> Option<String> {
        if buffer.starts_with(COMMAND_PREFIX) {
            self.execute_command(&buffer);
            return None;
        }

        match program(nom::types::CompleteStr(&buffer)) {
            Ok((_, program)) => {
                let mut bytes = program.to_bytes(&self.asm.symbols);
                self.vm.program.append(&mut bytes);
                self.vm.run_once();
                None
            }
            Err(e) => {
                self.send_message(format!("Unable to  parse input : {:?}", e));
                None
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

    fn quit(&mut self, _args: &[&str]) {
        self.send_message(format!("Farewell! Have a great day!"));
        std::process::exit(0);
    }

    fn history(&mut self, _args: &[&str]) {
        let mut results = vec![];
        for command in &self.command_buffer {
            results.push(command.clone());
        }
        self.send_message(format!("{:#?}", results));
        self.send_prompt();
    }

    fn program(&mut self, _args: &[&str]) {
        self.send_message(format!(
            "Listing instructions currently in VM's program vector:"
        ));
        let mut results = vec![];
        for instruction in &self.vm.program {
            results.push(instruction.clone())
        }
        self.send_message(format!("{:#?}", results));
        self.send_message(format!("End of Program Listing"));
        self.send_prompt();
    }

    fn registers(&mut self, _args: &[&str]) {
        self.send_message(format!("Listing registers and all contents:"));
        let mut results = vec![];
        for register in &self.vm.registers {
            results.push(register.clone());
        }
        self.send_message(format!("{:#?}", results));
        self.send_message(format!("End of Register Listing"));
        self.send_prompt();
    }

    fn clear_program(&mut self, _args: &[&str]) {
        self.send_message(format!("Clearing all program.."));
        self.vm.program.clear();
        self.send_message(format!("Done!"));
        self.send_prompt();
    }

    fn clear_registers(&mut self, _args: &[&str]) {
        self.send_message(format!("Setting all registers to 0"));
        for i in 0..self.vm.registers.len() {
            self.vm.registers[i] = 0;
        }
        self.send_message(format!("Done!"));
        self.send_prompt();
    }

    fn symbols(&mut self, _args: &[&str]) {
        let mut results = vec![];
        for symbol in &self.asm.symbols.symbols {
            results.push(symbol.clone());
        }
        self.send_message(format!("Listing symbols table:"));
        self.send_message(format!("{:#?}", results));
        self.send_message(format!("End of Symbols Listing"));
        self.send_prompt();
    }

    fn load_file(&mut self, _args: &[&str]) {
        let raw_content = self.get_data_from_load();
        match raw_content {
            Some(raw_content) => {
                let contents = self.asm.assemble(&raw_content);
                match contents {
                    Ok(mut assembled_program) => {
                        self.send_message(format!("Sending assembled program to VM"));
                        self.vm.program.append(&mut assembled_program);
                        self.vm.run();
                    }
                    Err(errors) => {
                        for error in errors {
                            self.send_message(format!("Unable to parse input: {}", error));
                            self.send_prompt();
                        }
                    }
                }
            }
            None => {}
        }
    }

    fn spawn(&mut self, _args: &[&str]) {
        let contents = self.get_data_from_load();
        if contents.is_none() {
            return;
        }
        match self.asm.assemble(&contents.unwrap()) {
            Ok(mut assembled_program) => {
                self.send_message(format!("Sending assembled program to VM"));
                self.vm.program.append(&mut assembled_program);
                println!("{:#?}", self.vm.program);
                self.scheduler.get_thread(self.vm.clone());
            }
            Err(errors) => {
                for err in errors {
                    self.send_message(format!("Unable to parse input: {}", err));
                    self.send_prompt();
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

    pub fn send_message(&mut self, msg: String) {
        match &self.tx_pipe {
            Some(pipe) => {
                pipe.send(msg + "\n");
            }
            None => {}
        }
    }
    pub fn send_prompt(&mut self) {
        match &self.tx_pipe {
            Some(pipe) => {
                pipe.send(PROMPT.to_owned());
            }
            None => {}
        }
    }
}
