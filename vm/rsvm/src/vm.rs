use chrono::prelude::*;
use std::{io::Cursor, vec};

use byteorder::{LittleEndian, ReadBytesExt};
use uuid::Uuid;

use crate::{
    assembler::{PIE_HEADER_LENGTH, PIE_HEADER_PREFIX},
    instruction::Opcode,
};

#[derive(Debug, Clone)]
pub enum VMEventType {
    Start,
    GracefulStop { code: u32 },
    Crash,
    Stop,
}

#[derive(Debug, Clone)]
pub struct VMEvent {
    event: VMEventType,
    at: DateTime<Utc>,
}

#[derive(Debug, Clone)]
pub struct VM {
    /// A unique idenfier of each VM isolate
    id: Uuid,
    events: Vec<VMEvent>,

    pub registers: [i32; 32],
    pc: usize, // program counter
    pub program: Vec<u8>,
    heap: Vec<u8>,
    ro_data: Vec<u8>,

    remainder: u32,   //  int left after divide
    equal_flag: bool, // the result of last comparison op
}

impl VM {
    pub fn new() -> VM {
        VM {
            registers: [0; 32],
            pc: 0,
            program: vec![],
            heap: vec![],
            remainder: 0,
            equal_flag: false,
            ro_data: vec![],
            id: Uuid::new_v4(),
            events: vec![],
        }
    }

    pub fn new_with_header() -> VM {
        let mut vm = Self::new();
        vm.program = VM::prepend_header(vm.program);
        vm
    }

    pub fn run(&mut self) -> u32 {
        self.events.push(VMEvent::new(VMEventType::Start));

        if !self.verify_hader() {
            self.events.push(VMEvent::new(VMEventType::Crash));
            error!("Header was incorrect");
            return 1; //Some(1);
        }

        self.pc = VM::get_header_offset() + self.get_starting_offset();

        let mut is_done = None;
        while is_done.is_none() {
            is_done = self.execute_instructions();
        }

        self.events.push(VMEvent::new(VMEventType::GracefulStop {
            code: is_done.unwrap(),
        }));
        0 //Some(0)
    }

    pub fn run_once(&mut self) {
        self.execute_instructions();
    }

    pub fn add_byte(&mut self, b: u8) {
        self.program.push(b);
    }
    pub fn add_bytes(&mut self, mut bytes: Vec<u8>) {
        self.program.append(&mut bytes);
    }

    fn execute_instructions(&mut self) -> Option<u32> {
        if self.pc >= self.program.len() {
            return Some(1);
        }

        match self.decode_opcode() {
            Opcode::LOAD => {
                let register = self.next_8_bits() as usize;
                let number = self.next_16_bits() as u16;
                self.registers[register] = number as i32;
            }

            Opcode::ADD => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.registers[self.next_8_bits() as usize] = reg1 + reg2;
            }
            Opcode::SUB => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.registers[self.next_8_bits() as usize] = reg1 - reg2;
            }
            Opcode::MUL => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.registers[self.next_8_bits() as usize] = reg1 * reg2;
            }
            Opcode::DIV => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.registers[self.next_8_bits() as usize] = reg1 / reg2;
                self.remainder = (reg1 % reg2) as u32;
            }
            Opcode::INC => {
                let reg = self.next_8_bits();
                self.registers[reg as usize] += 1;
                self.next_16_bits(); //eat
            }
            Opcode::DEC => {
                let reg = self.next_8_bits();
                self.registers[reg as usize] -= 1;
                self.next_16_bits(); //eat
            }

            Opcode::EQ => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.equal_flag = if reg1 == reg2 { true } else { false };
                self.next_8_bits(); // eat it for mips or other isc write into register , we use the self.equal_flag
            }
            Opcode::NEQ => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.equal_flag = if reg1 != reg2 { true } else { false };
                self.next_8_bits(); // eat it for mips or other isc write into register , we use the self.equal_flag
            }
            Opcode::GT => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.equal_flag = if reg1 > reg2 { true } else { false };
                self.next_8_bits(); // eat it for mips or other isc write into register , we use the self.equal_flag
            }
            Opcode::LT => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.equal_flag = if reg1 < reg2 { true } else { false };
                self.next_8_bits(); // eat it for mips or other isc write into register , we use the self.equal_flag
            }
            Opcode::GTE => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.equal_flag = if reg1 >= reg2 { true } else { false };
                self.next_8_bits(); // eat it for mips or other isc write into register , we use the self.equal_flag
            }
            Opcode::LTE => {
                let (reg1, reg2) = self.binary_operaters_value();
                self.equal_flag = if reg1 <= reg2 { true } else { false };
                self.next_8_bits(); // eat it for mips or other isc write into register , we use the self.equal_flag
            }
            Opcode::JEQ => {
                let reg = self.next_8_bits() as usize;
                let target = self.registers[reg];
                if self.equal_flag {
                    self.pc = target as usize;
                }
            }
            Opcode::JNEQ => {
                let reg = self.next_8_bits() as usize;
                let target = self.registers[reg];
                if !self.equal_flag {
                    self.pc = target as usize;
                }
            }

            Opcode::JMPB => {
                let target = self.registers[self.next_8_bits() as usize];
                self.pc -= target as usize;
            }
            Opcode::JMP => {
                let target = self.registers[self.next_8_bits() as usize];
                self.pc = target as usize;
            }
            Opcode::JMPF => {
                println!("jumpf");
                let target = self.registers[self.next_8_bits() as usize];
                self.pc += target as usize;
            }

            Opcode::NOP => {
                self.next_16_bits();
                self.next_8_bits();
            }
            Opcode::ALOC => {
                let reg = self.next_8_bits() as usize;
                let num_bytes = self.registers[reg];
                let new_heap_size = self.heap.len() as i32 + num_bytes;
                self.heap.resize(new_heap_size as usize, 0);
            }
            Opcode::PRTS => {
                let starting_offset = self.next_16_bits() as usize;
                let mut ending_offset = starting_offset;
                let slice = self.ro_data.as_slice();
                // trance the string till '\0'
                while slice[ending_offset] != 0 {
                    ending_offset += 1;
                }
                let ret = std::str::from_utf8(&slice[starting_offset..ending_offset]);
                match ret {
                    Ok(s) => print!("{}", s),
                    Err(e) => {
                        println!("Error decoding string for prts instruction: {:#?}", e)
                    }
                }
            }

            Opcode::HLT => {
                println!("HLT encountered");
                return None;
            }

            // Opcode::IGL => {
            //     println!("Illegal instruction encounterd");
            //     return 1;
            // }
            _ => {
                println!(
                    "Unrecognized opcode: {} found! Terminating!",
                    self.program[self.pc]
                );
                return None;
            }
        }
        None
    }

    fn decode_opcode(&mut self) -> Opcode {
        let opcode = Opcode::from(self.program[self.pc]);
        self.pc += 1;
        opcode
    }

    fn next_8_bits(&mut self) -> u8 {
        let result = self.program[self.pc];
        self.pc += 1;
        result
    }
    fn next_16_bits(&mut self) -> u16 {
        let result = ((self.program[self.pc] as u16) << 8) | self.program[self.pc + 1] as u16;
        self.pc += 2;
        result
    }

    fn binary_operaters_value(&mut self) -> (i32, i32) {
        (
            self.registers[self.next_8_bits() as usize],
            self.registers[self.next_8_bits() as usize],
        )
    }

    fn verify_hader(&self) -> bool {
        if self.program[0..4] != PIE_HEADER_PREFIX {
            return false;
        }
        true
    }

    // just add the header bytes before program
    fn prepend_header(mut b: Vec<u8>) -> Vec<u8> {
        let mut prepension = vec![];
        for byte in PIE_HEADER_PREFIX {
            prepension.push(byte.clone());
        }
        // 4 bytes for telling the vm code starting offset
        while prepension.len() < PIE_HEADER_LENGTH + 4 {
            prepension.push(0);
        }
        prepension.append(&mut b);
        prepension
    }

    pub fn get_header_offset() -> usize {
        // 4 bytes of pie header | ...64 empty | 4b for code start -> currently 64+4
        PIE_HEADER_LENGTH + 4
    }

    fn get_starting_offset(&self) -> usize {
        let mut cursor = Cursor::new(&self.program[64..68]);
        cursor.read_u32::<LittleEndian>().unwrap() as usize
    }
}

impl VMEvent {
    fn new(event_type: VMEventType) -> VMEvent {
        VMEvent {
            event: event_type,
            at: Utc::now(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::vec;

    #[test]
    fn test_create_vm() {
        let vm = VM::new();
        vm.registers
            .iter()
            .for_each(|register| assert_eq!(register, &0))
    }

    #[test]
    fn test_opcode_hlt() {
        let mut test_vm = VM::new();
        let test_bytes = vec![Opcode::HLT.into(), 0, 0, 0];
        test_vm.program = VM::prepend_header(test_bytes);
        test_vm.run();
        assert_eq!(test_vm.pc, VM::get_header_offset() + 1);
    }

    #[test]
    fn test_opcode_igl() {
        let mut test_vm = VM::new_with_header();
        let mut test_bytes = vec![Opcode::IGL.into(), 0, 0, 0];
        test_vm.program.append(&mut test_bytes);
        test_vm.run();
        assert_eq!(test_vm.pc, VM::get_header_offset() + 1);
    }

    #[test]
    fn test_opcode_load() {
        let mut test_vm = VM::new_with_header();
        test_vm
            .program
            .append(&mut vec![Opcode::LOAD.into(), 0, 1, 244]);
        test_vm.run();
        assert_eq!(test_vm.registers[0], 500);
    }

    #[test]
    fn test_opcode_jmp() {
        let mut test_vm = VM::new();
        test_vm.program = vec![80, 0, 0, 0];
        test_vm.registers[0] = 1;
        test_vm.run_once();
        assert_eq!(test_vm.pc, 1);
    }

    #[test]
    fn test_opcode_eq() {
        let mut test_vm = VM::new();
        test_vm.registers[0] = 10;
        test_vm.registers[1] = 10;
        test_vm.program = vec![Opcode::EQ.into(), 0, 1, 255];
        test_vm.run_once();
        assert_eq!(test_vm.equal_flag, true);

        test_vm.pc = 0;
        test_vm.registers[1] = 20;
        test_vm.run_once();
        assert_eq!(test_vm.equal_flag, false);
    }

    #[test]
    fn test_opcode_jeq() {
        let mut test_vm = VM::new();
        test_vm.registers[0] = 7;
        test_vm.equal_flag = true;
        test_vm.program = vec![Opcode::JEQ.into(), 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0];
        test_vm.run_once();
        assert_eq!(test_vm.pc, 7);
    }

    #[test]
    fn test_opcode_jmp_relatively() {
        {
            let mut vm = VM::new();
            vm.registers[0] = 2;

            vm.program = vec![Opcode::JMPF.into(), 0, 0, 0, 6, 0, 0, 0];
            vm.run_once();
            assert_eq!(vm.pc, 4);
        }
        {
            let mut vm = VM::new();
            vm.registers[0] = 5;
            vm.pc = 3;

            // 3 +1 +1 -5=   0
            vm.program = vec![0, 1, 2, Opcode::JMPB.into(), 0, 5, 6, 7];
            vm.run_once();
            assert_eq!(vm.pc, 0);
        }
    }

    #[test]
    fn test_opcode_aloc() {
        let mut vm = VM::new();
        vm.registers[0] = 1024;
        vm.program = vec![Opcode::ALOC.into(), 0, 0, 0];
        vm.run_once();
        assert_eq!(vm.heap.len(), 1024);
    }

    #[test]
    fn test_opcode_inc_and_dec() {
        let mut vm = VM::new();
        vm.registers[0] = 1024;
        vm.program = vec![Opcode::INC.into(), 0, 0, 0, Opcode::DEC.into(), 0, 0, 0];
        vm.run_once();
        assert_eq!(vm.registers[0], 1025);
        vm.run_once();
        assert_eq!(vm.registers[0], 1024);
    }
    #[test]
    fn test_opcode_mul() {
        let mut vm = VM::new_with_header();
        vm.program.append(&mut vec![Opcode::MUL.into(), 0, 1, 2]);
        vm.registers[0] = 25;
        vm.registers[1] = 2;
        vm.run();
        assert_eq!(vm.registers[2], 50)
    }

    #[test]
    fn test_opcode_prts() {
        let mut vm = VM::new();
        vm.ro_data
            .append(&mut vec![72, 101, 108, 108, 101, '\n' as u8, 0]);
        vm.program = vec![Opcode::PRTS.into(), 0, 0, 0];
        vm.run_once();
    }
}
