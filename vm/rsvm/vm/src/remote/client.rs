use std::io::{BufRead, BufReader, BufWriter, Write};
use std::net::TcpStream;
use std::thread::{self, sleep};

use crate::repl;

pub struct Client {
    reader: BufReader<TcpStream>,
    writer: BufWriter<TcpStream>,
    raw_stream: TcpStream,
    repl: repl::REPL,
}

impl Client {
    pub fn new(stream: TcpStream) -> Client {
        // TODO: Handle this better
        let reader = stream.try_clone().unwrap();
        let writer = stream.try_clone().unwrap();
        let mut repl = repl::REPL::new();

        Client {
            reader: BufReader::new(reader),
            writer: BufWriter::new(writer),
            raw_stream: stream,
            repl: repl,
        }
    }

    pub fn run(&mut self) {
        self.receive_loop();
        let mut buf = String::new();
        let banner = repl::REMOTE_BANNER.to_owned() + "\n" + repl::PROMPT;
        self.w(&banner);
        loop {
            match self.reader.read_line(&mut buf) {
                Ok(_) => {
                    buf.trim_end();
                    self.repl.run_single(&buf);
                    buf.clear();
                }
                Err(e) => println!("Errror receiving: {:#?}", e),
            }
        }
    }

    fn write_prompt(&mut self) {
        self.w(repl::PROMPT);
    }

    fn w(&mut self, msg: &str) -> bool {
        match self.writer.write_all(msg.as_bytes()) {
            Ok(_) => match self.writer.flush() {
                Ok(_) => true,
                Err(e) => {
                    println!("Error flushing to client: {}", e);
                    false
                }
            },
            Err(e) => {
                println!("Error writing to client: {}", e);
                false
            }
        }
    }

    // currently read the msg from the sender of tx_pipe of repl (Message pipe from repl)
    // THen do write/echo to client by tcp stream
    fn receive_loop(&mut self) {
        let rx = self.repl.rx_pipe.take();
        let mut writer = self.raw_stream.try_clone().unwrap();
        let t = thread::spawn(move || {
            let chan = rx.unwrap();
            loop {
                match chan.recv() {
                    Ok(msg) => {
                        writer.write_all(msg.as_bytes());
                        writer.flush();
                    }
                    Err(e) => {}
                }
            }
        });
    }
}
