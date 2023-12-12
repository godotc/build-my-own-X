use super::parser::program_parser::program;
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

    pub fn compile(&self) {
        // self.visit_token(self.assembly);
    }
}

impl Visitor for Compiler {
    fn visit_token(&mut self, node: &crate::token::Token) {
        match node {
            Token::AdditionOperator => {
                println!("Visiting AdditionOperator");
                let result_reg = self.free_registers.pop().unwrap();
                let left_reg = self.used_registers.pop().unwrap();
                let right_reg = self.used_registers.pop().unwrap();
                let line = format!("ADD ${} ${} ${}", left_reg, right_reg, result_reg);
                self.assembly.push(line);
                self.used_registers.push(result_reg);
                self.free_registers.push(left_reg);
                self.free_registers.push(right_reg);
            }
            Token::SubtractionOperator => {
                println!("Visiting SubstractionOperator");
                let result_reg = self.free_registers.pop().unwrap();
                let left_reg = self.used_registers.pop().unwrap();
                let right_reg = self.used_registers.pop().unwrap();
                let line = format!("SUB ${} ${} ${}", right_reg, left_reg, result_reg);
                self.assembly.push(line);
                self.used_registers.push(result_reg);
                self.free_registers.push(left_reg);
                self.free_registers.push(right_reg);
            }
            Token::MultiplicationOperator => {
                println!("Visiting MultiplicationOperator");
                let result_reg = self.free_registers.pop().unwrap();
                let left_reg = self.used_registers.pop().unwrap();
                let right_reg = self.used_registers.pop().unwrap();
                let line = format!("MUL ${} ${} ${}", right_reg, left_reg, result_reg);
                self.assembly.push(line);
                self.used_registers.push(result_reg);
                self.free_registers.push(left_reg);
                self.free_registers.push(right_reg);
            }
            Token::DivisionOperator => {
                println!("Visiting DivisionOperator");
                let result_reg = self.free_registers.pop().unwrap();
                let left_reg = self.used_registers.pop().unwrap();
                let right_reg = self.used_registers.pop().unwrap();
                let line = format!("DIV ${} ${} ${}", right_reg, left_reg, result_reg);
                self.assembly.push(line);
                self.used_registers.push(result_reg);
                self.free_registers.push(left_reg);
                self.free_registers.push(right_reg);
            }

            Token::Integer { value } => {
                println!("Visited integer: {:#?}", value);
                let next_reg = self.free_registers.pop().unwrap();
                self.used_registers.push(next_reg);
                let line = format!("LOAD ${} #{}", next_reg, value);
                self.assembly.push(line);
            }
            Token::Float { value } => {
                println!("Visited float: {:#?}", value);
                let next_reg = self.free_registers.pop().unwrap();
                self.used_registers.push(next_reg);
                let line = format!("LOAD ${} #{}", next_reg, value);
                self.assembly.push(line);
            }
            //
            Token::Factor { value } => {
                println!("Begin visiting factor");
                self.visit_token(&value);
                println!("Done visiting factor");
            }
            Token::Term { left, right } => {
                println!("Begin visiting term");
                // 1
                self.visit_token(&left);
                // *1*2*3...
                for expr in right {
                    self.visit_token(&expr.1);
                    self.visit_token(&expr.0);
                }
                println!("Done visiting term");
            }

            Token::Expression { left, right } => {
                println!("Begin visiting expression");
                // post order traversal
                // (4*3)-1
                self.visit_token(&left);
                for expr in right {
                    self.visit_token(&expr.1);
                    self.visit_token(&expr.0);
                }
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

    fn generate_test_program(str: &str) -> Token {
        let source = CompleteStr(str);
        let (_, tree) = program(source).unwrap();
        tree
    }

    #[test]
    fn test_visit_addition_token() {
        let mut compiler = Compiler::new();
        let test_program = generate_test_program("1+2");
        compiler.visit_token(&test_program);
    }

    #[test]
    fn test_nested_operators() {
        let mut compiler = Compiler::new();
        let test_program = generate_test_program("(4*3)-1");
        println!("{:#?}", test_program);
        compiler.visit_token(&test_program);
        println!("{:#?}", compiler.assembly);
        let bytecode = compiler.compile();
    }
}
