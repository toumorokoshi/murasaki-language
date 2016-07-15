pub mod yaml;
mod assignment;
mod declaration;
mod binop;
mod condition;
pub use self::assignment::Assignment;
pub use self::declaration::Declaration;
pub use self::binop::{BinOp, BinaryOperator};
pub use self::condition::Condition;

#[derive(Clone, PartialEq, Debug)]
pub struct FunctionDecl {
    pub name: String,
    pub statements: Vec<Box<Statement>>,
    pub typ: String,
}

#[derive(Clone, PartialEq, Debug)]
pub enum Expression {
    ConstInt{value: i64},
    ConstFloat{value: f64},
    ConstString{value: String},
    BinOp(BinOp),
    Symbol(String),
    Call{name: String, arg: Box<Expression>},
    Condition(Condition)
}

#[derive(Clone, Debug, PartialEq)]
pub enum Statement {
    FunctionDecl(FunctionDecl),
    Return(Expression),
    Expr(Expression),
    Declaration(Declaration),
}

pub type Statements = Vec<Box<Statement>>;