use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    types::{ToShape, Type},
    walk::{CommonVisitor, ProgramVisitor},
    Identify, TypeOf,
};
use std::collections::HashMap;

pub const fn can_render(x: &ast::typed::InnerType) -> bool {
    matches!(
        x,
        Type::Nil | Type::Boolean | Type::Integer | Type::Float | Type::String | Type::Element
    )
}

pub fn analyze(
    x: &ast::Component<
        <Visitor as ProgramVisitor>::Component,
        <Visitor as ProgramVisitor>::Expression,
        <Visitor as ProgramVisitor>::Attribute,
    >,
    ctx: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::Component::Text(_) => None,

        ast::Component::Expression(x) => {
            (!can_render(x.type_of())).then_some(vec![Error::NotRenderable(*x.id())])
        }

        ast::Component::Fragment(_) => None,

        ast::Component::ClosedElement(start_tag @ end_tag, attributes)
        | ast::Component::OpenElement {
            start_tag,
            end_tag,
            attributes,
            ..
        } => {
            let mut errors = vec![];

            if start_tag != end_tag {
                errors.push(Error::ComponentTypo(start_tag.clone(), end_tag.clone()));
            }

            if let Type::View(parameters) = ctx.type_of() {
                let mut unsatisfied_parameters = parameters
                    .iter()
                    .map(|x| (x.name().to_owned(), x))
                    .collect::<HashMap<_, _>>();
                let mut unexpected_attributes = vec![];

                for attribute in attributes {
                    let name = attribute.0.value().name();

                    if let Some(parameter) = unsatisfied_parameters.remove(name) {
                        let parameter_type = parameter.value().type_of().to_shape();
                        let argument_type = attribute.0.type_of().to_shape();

                        if parameter_type != argument_type {
                            errors.push(Error::AttributeRejected(
                                *parameter.value().id(),
                                *attribute.0.id(),
                            ));
                        }
                    } else {
                        unexpected_attributes.push(attribute);
                    }
                }

                let mut sorted_parameters = unsatisfied_parameters.into_iter().collect::<Vec<_>>();
                sorted_parameters.sort_by(|l, r| l.0.cmp(&r.0));

                errors.extend(sorted_parameters.into_iter().filter_map(|(_, x)| {
                    x.is_required()
                        .then_some(Error::MissingAttribute(*x.value().id()))
                }));

                errors.extend(
                    unexpected_attributes
                        .into_iter()
                        .map(|x| Error::UnexpectedAttribute(x.0.value().name().to_owned())),
                );
            } else {
                errors.push(Error::InvalidComponent(start_tag.clone()));
            }

            (!errors.is_empty()).then_some(errors)
        }
    }
}
