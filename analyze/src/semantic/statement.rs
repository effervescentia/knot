use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub fn analyze(x: &ast::Statement<<Visitor as Visit>::Expression>) -> Option<Vec<Error>> {
    match x {
        _ => (),
    }

    None
}
