use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub const fn analyze(
    _: &ast::Module<<Visitor as Visit>::Import, <Visitor as Visit>::Declaration>,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    None
}
