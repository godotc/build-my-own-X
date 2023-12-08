use nom::print;

use super::program_parser::program;
use crate::token::Token;
use crate::visitor::Visitor;

pub struct Compiler {
    free_registers: Vec<u8>,
    used_registers: Vec<u8>,
    assembly: Vec<String>,
}

impl Compiler {
    pub fn new() -> Compiler {
        let mut free_registers = vec![];
        for i in 0..32 {
            free_registers.push(i);
        }
        free_registers.reverse();
        Compiler {
            free_registers: free_registers,
            used_registers: vec![],
            assembly: vec![],
        }
    }

    fn requset_free_reg(&mut self) -> u8 {
        self.free_registers.pop().unwrap()
    }
    fn release_used_reg(&mut self) -> u8 {
        self.used_registers.pop().unwrap()
    }
}

impl Visitor for Compiler {
    fn visit_token(&mut self, node: &crate::token::Token) {
        match node {
            Token::AdditionOperator => {
                println!("Visiting AdditionOperator");
                let result_reg = self.requset_free_reg();
                let left_reg = self.release_used_reg();
                let right_reg = self.release_used_reg();
                let line = format!("ADD ${} ${} ${}", left_reg, right_reg, result_reg);
                self.assembly.push(line);
                self.free_registers.push(left_reg);
                self.free_registers.push(right_reg);
            }
            Token::SubtractionOperator => todo!(),
            Token::MultiplicationOperator => todo!(),
            Token::DivisionOperator => todo!(),
            Token::Integer { value } => {
                println!("Visited integer: {:#?}", value);
                let next_reg = self.requset_free_reg();
                let line = format!("LOAD ${} #{}", next_reg, value);
                self.release_used_reg();
                self.assembly.push(line);
            }

            Token::Expression { left, op, right } => {
                println!("Begin visiting expression");
                // post order traversal
                self.visit_token(&left);
                self.visit_token(&right);
                self.visit_token(&op);
                println!("Done visiting expression");
            }
            Token::Program { expressions } => {
                println!("Visiting program");
                for exp in expressions {
                    self.visit_token(exp);
                }
                println!("Done visiting program");
            }
        }
    }
}

mod tests {
    use super::*;
    use nom::types::CompleteStr;

    fn generate_test_program() -> Token {
        let source = CompleteStr("1+2");
        let (_, tree) = program(source).unwrap();
        tree
    }

    #[test]
    fn test_visit_addition_token() {
        let mut compiler = Compiler::new();
        let test_program = generate_test_program();
        compiler.visit_token(&test_program);
    }
}
