mod instruction;
pub mod repl;
pub mod vm;

pub mod assembler;

#[macro_use]
extern crate nom;

extern crate clap;
use clap::{App, Arg, SubCommand};

#[macro_use]
extern crate log;
