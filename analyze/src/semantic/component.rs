use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::Visit};

pub fn analyze(
    x: &ast::Component<<Visitor as Visit>::Component, <Visitor as Visit>::Expression>,
) -> Option<Vec<Error>> {
    match x {
        ast::Component::Text(x) => (),

        ast::Component::Expression(x) => (),

        ast::Component::Fragment(children) => (),

        ast::Component::ClosedElement(tag, children) => (),

        ast::Component::OpenElement { .. } => (),
    }

    None
}
