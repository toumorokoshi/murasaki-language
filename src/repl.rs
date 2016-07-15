use super::codegen;
use super::vm;
use super::ast;
use std;
use std::io::{self, Write};
use super::peg_grammar;

pub fn start_repl() -> Result<(), std::io::Error> {
    println!("Greyhawk 0.0.3");
    let mut vm_instance = vm::VM::new();
    try!(repl(&mut vm_instance));
    Ok(())
}

fn repl(vm_instance: &mut vm::VM) -> Result<(), std::io::Error> {
    loop {
        try!(std::io::stdout().write(b">>> "));
        try!(std::io::stdout().flush());
        let mut input = String::new();
        io::stdin().read_line(&mut input).ok().expect("Failed to read line");
        match peg_grammar::module(&input) {
            Ok(mut statement_list) => {
                convert_last_expression_to_return(&mut statement_list);
                let function = codegen::generate_ops(&statement_list);
                let object = vm_instance.execute_function(&function, &[]);
                vm::print(&[object]);
            },
            Err(err) => {
                println!("{}", err);
            }
        }
    }
}

fn convert_last_expression_to_return(statements: &mut ast::Statements) {
    let last = statements.pop();
    if let Some(s) = last {
        if let box ast::Statement::Expr(e) = s {
            statements.push(Box::new(ast::Statement::Return(e)));
        }
    }
}