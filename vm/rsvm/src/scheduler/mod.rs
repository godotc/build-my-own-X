use std::thread;

use crate::vm::VM;

#[derive(Debug, Default, Clone)]
pub struct Scheduler {
    next_pid: u32,
    max_pid: u32,
}

impl Scheduler {
    pub fn new() -> Scheduler {
        Scheduler {
            next_pid: 0,
            max_pid: 50000,
        }
    }

    pub(crate) fn get_thread(&self, mut vm: VM) -> thread::JoinHandle<u32> {
        // thread::spawn(|| match vm.run() {
        //     Some(n) => n,
        //     None => 1,
        // })
        thread::spawn(move || vm.run())
    }
}
