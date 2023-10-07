use std::result;

use crate::instruction::Opcode;

#[derive(Debug)]
pub struct VM {
    registers: [i32; 32],
    pc: usize, // program counter
    program: Vec<u8>,
    remainder: u32,
}

impl VM {
    pub fn new() -> VM {
        VM {
            registers: [0; 32],
            pc: 0,
            program: vec![],
            remainder: 0,
        }
    }

    pub fn run(&mut self) {
        let mut is_done = false;
        while !is_done {
            is_done = self.execute_instructions();
        }
    }

    fn execute_instructions(&mut self) -> bool {
        if self.pc >= self.program.len() {
            return false;
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

            Opcode::JMP => {
                let target = self.registers[self.next_8_bits() as usize];
                self.pc = target as usize;
            }

            Opcode::HLT => {
                println!("HLT encountered");
                return false;
            }
            _ => {
                println!("Unrecognized opcde found! Terminating!");
                return false;
            }
        }
        true
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
}

#[cfg(test)]
mod tests {
    use super::*;

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
        let test_bytes = vec![0, 0, 0, 0];
        test_vm.program = test_bytes;
        test_vm.run();
        assert_eq!(test_vm.pc, 1);
    }

    #[test]
    fn test_opcode_ilg() {
        let mut test_vm = VM::new();
        let test_bytes = vec![200, 0, 0, 0];
        test_vm.program = test_bytes;
        test_vm.run();
        assert_eq!(test_vm.pc, 1);
    }

    #[test]
    fn test_opcode_load() {
        let mut test_vm = VM::new();
        let test_bytes = vec![0, 0, 1, 244]; // 500 in little endian
        test_vm.program = test_bytes;
        test_vm.run();
        assert_eq!(test_vm.registers[0], 500);
    }
}
