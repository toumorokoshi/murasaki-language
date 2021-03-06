use super::Expression;
use std::fmt;

#[derive(Clone, PartialEq, Debug)]
pub enum BinaryOperator {
    Plus,
    Sub,
    Mul,
    Div,
    Neq,
    Eq,
    Leq,
    Le,
    Ge,
    Geq
}

impl fmt::Display for BinaryOperator {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            &BinaryOperator::Plus => f.write_str("+"),
            &BinaryOperator::Sub  => f.write_str("-"),
            &BinaryOperator::Mul  => f.write_str("/"),
            &BinaryOperator::Div  => f.write_str("*"),
            &BinaryOperator::Neq => f.write_str("!="),
            &BinaryOperator::Eq => f.write_str("=="),
            &BinaryOperator::Le => f.write_str("<"),
            &BinaryOperator::Leq => f.write_str("<="),
            &BinaryOperator::Ge => f.write_str(">"),
            &BinaryOperator::Geq => f.write_str(">="),
        }
    }
}

#[derive(Clone, PartialEq, Debug)]
pub struct BinOp {
    pub op: BinaryOperator,
    pub left: Box<Expression>,
    pub right: Box<Expression>
}
