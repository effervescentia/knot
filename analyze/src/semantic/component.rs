use super::Visitor;
use crate::error::Error;
use lang::{ast, types::Type, walk::Visit, Identify, TypeOf};

pub fn analyze(
    x: &ast::Component<<Visitor as Visit>::Component, <Visitor as Visit>::Expression>,
    _: &<Visitor as Visit>::Context,
) -> Option<Vec<Error>> {
    match x {
        ast::Component::Text(_) => None,

        ast::Component::Expression(x) => match x.type_of() {
            Type::Nil
            | Type::Boolean
            | Type::Integer
            | Type::Float
            | Type::String
            | Type::Element => None,

            _ => Some(vec![Error::NotRenderable(*x.id())]),
        },

        ast::Component::Fragment(_) => None,

        ast::Component::ClosedElement(..) => None,

        ast::Component::OpenElement { .. } => None,
    }
}
