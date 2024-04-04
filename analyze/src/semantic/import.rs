use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub const fn analyze(_: &ast::Import, _: &<Visitor as Visit>::Context) -> Option<Vec<Error>> {
    None
}
