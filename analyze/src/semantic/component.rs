use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub fn analyze(
    x: &ast::Component<<Visitor as Visit>::Component, <Visitor as Visit>::Expression>,
) -> Option<Vec<Error>> {
    match x {
        _ => (),
    }

    None
}
