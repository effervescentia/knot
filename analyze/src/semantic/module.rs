use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub fn analyze(
    x: &ast::Expression<
        <Visitor as Visit>::Expression,
        <Visitor as Visit>::Statement,
        <Visitor as Visit>::Component,
    >,
) -> Option<Vec<Error>> {
    match x {
        ast::Expression::BinaryOperation(op, lhs, rhs) => (),

        ast::Expression::UnaryOperation(op, x) => (),

        _ => (),
    }

    None
}
