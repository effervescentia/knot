use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub const fn analyze(
    x: &ast::Attribute<<Visitor as Visit>::Expression>,
    _: &<Visitor as Visit>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::Attribute::Punned(_) => None,

        ast::Attribute::Explicit(..) => None,
    }
}
