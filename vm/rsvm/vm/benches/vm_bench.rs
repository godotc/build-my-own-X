#[macro_use]
extern crate criterion;
extern crate vm;

use criterion::Criterion;
use vm::assembler::{PIE_HEADER_LENGTH, PIE_HEADER_PREFIX};
use vm::vm::VM;

mod arithmetic {

    use vm::instruction::Opcode;

    use super::*;

    fn execute_add(c: &mut Criterion) {
        let clos = {
            let mut test_vm = VM::new_with_non_zero_registers();
            test_vm.program = vec![Opcode::ADD.into(), 0, 1, 2];
            test_vm.run_once();
        };

        c.bench_function("execute_add", move |b| b.iter(|| clos));
    }
    fn execute_sub(c: &mut Criterion) {
        let clos = {
            let mut test_vm = VM::new_with_non_zero_registers();
            test_vm.program = vec![Opcode::SUB.into(), 0, 1, 2];
            test_vm.run_once();
        };

        c.bench_function("execute_sub", move |b| b.iter(|| clos));
    }
    fn execute_mul(c: &mut Criterion) {
        let clos = {
            let mut test_vm = VM::new_with_non_zero_registers();
            test_vm.program = vec![Opcode::MUL.into(), 0, 1, 2];
            test_vm.run_once();
        };

        c.bench_function("execute_mul", move |b| b.iter(|| clos));
    }
    fn execute_div(c: &mut Criterion) {
        let clos = {
            let mut test_vm = VM::new_with_non_zero_registers();
            test_vm.program = vec![Opcode::DIV.into(), 0, 1, 2];
            test_vm.run_once();
        };

        c.bench_function("execute_div", move |b| b.iter(|| clos));
    }

    criterion_group! {
       name =arithmetic;
       config = Criterion::default();
       targets = execute_add, execute_sub, execute_mul, execute_div,
    }
}

criterion_main!(arithmetic::arithmetic);
