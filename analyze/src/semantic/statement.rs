use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub const fn analyze(
    x: &ast::Statement<<Visitor as Visit>::Expression>,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    match x {
        ast::Statement::Expression(_) => None,

        ast::Statement::Variable(..) => None,
    }
}
