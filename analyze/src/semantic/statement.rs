use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub fn analyze(x: &ast::Statement<<Visitor as Visit>::Expression>) -> Option<Vec<Error>> {
    match x {
        ast::Statement::Expression(x) => (),

        ast::Statement::Variable(name, x) => (),
    }

    None
}
