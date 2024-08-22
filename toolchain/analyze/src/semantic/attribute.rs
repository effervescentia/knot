use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    walk::{CommonVisitor, ProgramVisitor},
};

pub const fn analyze(
    x: &ast::Attribute<<Visitor as ProgramVisitor>::Expression>,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::Attribute::Punned(_) => None,

        ast::Attribute::Explicit(..) => None,
    }
}
