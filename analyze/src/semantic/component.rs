use super::Visitor;
use crate::error::Error;
use lang::{ast, types::Type, walk::Visit, Identify, TypeOf};

pub const fn can_render(x: &ast::typed::InnerType) -> bool {
    matches!(
        x,
        Type::Nil | Type::Boolean | Type::Integer | Type::Float | Type::String | Type::Element
    )
}

pub fn analyze(
    x: &ast::Component<<Visitor as Visit>::Component, <Visitor as Visit>::Expression>,
    ctx: &<Visitor as Visit>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::Component::Text(_) => None,

        ast::Component::Expression(x) => {
            (!can_render(x.type_of())).then_some(vec![Error::NotRenderable(*x.id())])
        }

        ast::Component::Fragment(_) => None,

        ast::Component::ClosedElement(tag, ..)
        | ast::Component::OpenElement { start_tag: tag, .. }
            if !matches!(ctx.type_of(), Type::View(_)) =>
        {
            Some(vec![Error::InvalidComponent(tag.clone())])
        }

        ast::Component::ClosedElement(..) => None,

        ast::Component::OpenElement {
            start_tag, end_tag, ..
        } => (start_tag != end_tag).then_some(vec![Error::ComponentTypo(
            start_tag.clone(),
            end_tag.clone(),
        )]),
    }
}
