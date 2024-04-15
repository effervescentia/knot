use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::CommonVisitor};

pub const fn analyze(
    _: &ast::Import,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    None
}
