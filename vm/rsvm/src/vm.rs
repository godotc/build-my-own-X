use crate::instruction::Opcode;

#[derive(Debug)]
pub struct VM {
    pub registers: [i32; 32],
    pub pc: usize, // program counter
    pub program: Vec<u8>,
    pub remainder: u32,   // divide int left
    pub equal_flag: bool, // the result of last comparison op
}

impl VM {
    pub fn new() -> VM {
        VM {
            registers: [0; 32],
            pc: 0,
            program: vec![],
            remainder: 0,
            equal_flag: false,
        }
    }

    pub fn run(&mut self) {
        let mut is_done = false;
        while !is_done {
            is_done = self.execute_instructions();
        }
    }

    pub fn run_once(&mut self) {
        self.execute_instructions();
    }

    pub fn add_byte(&mut self, b: u8) {
        self.program.push(b);
    }

    fn execute_instructions(&mut self) -> bool {
        if self.pc >= self.program.len() {
            return true;
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

            Opcode::HLT => {
                println!("HLT encountered");
                return true;
            }

            _ => {
                println!(
                    "Unrecognized opcode: {} found! Terminating!",
                    self.program[self.pc]
                );
                return true;
            }
        }
        false
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
        let test_bytes = vec![99, 0, 0, 0];
        test_vm.program = test_bytes;
        test_vm.run();
        assert_eq!(test_vm.pc, 1);
    }

    #[test]
    fn test_opcode_ilg() {
        let mut test_vm = VM::new();
        let test_bytes = vec![Opcode::IGL.into(), 0, 0, 0];
        test_vm.program = test_bytes;
        test_vm.run();
        assert_eq!(test_vm.pc, 1);
    }

    #[test]
    fn test_opcode_load() {
        let mut test_vm = VM::new();
        test_vm.program = vec![0, 0, 1, 244];
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
}
