use std::collections::HashMap;

use super::Visitor;
use crate::error::Error;
use lang::{
    ast,
    types::{ToShape, Type},
    walk::Visit,
    Identify, TypeOf,
};

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

            match ctx.type_of() {
                Type::View(parameters) => {
                    let mut unsatisfied_parameters = parameters
                        .iter()
                        .map(|x| (x.name().to_owned(), x))
                        .collect::<HashMap<_, _>>();
                    let mut unexpected_attributes = vec![];

                    for attribute @ (name, value) in attributes {
                        if let Some(parameter) = unsatisfied_parameters.remove(name) {
                            let parameter_type = parameter.value().type_of().to_shape();

                            let (argument_id, argument_type) = if let Some(expression) = value {
                                (expression.id(), expression.type_of().to_shape())
                            } else {
                            };

                            if parameter_type != expression.type_of().to_shape() {
                                errors.push(Error::AttributeRejected(
                                    *parameter.value().id(),
                                    *expression.id(),
                                ));
                            }
                        } else {
                            unexpected_attributes.push(attribute);
                        }
                    }

                    errors.extend(
                        unsatisfied_parameters
                            .values()
                            .map(|x| Error::MissingAttribute(*x.value().id())),
                    );

                    errors.extend(
                        unexpected_attributes
                            .iter()
                            .map(|(x, _)| Error::UnexpectedAttribute(x.clone())),
                    );

                    ()
                }

                _ => errors.push(Error::InvalidComponent(start_tag.clone())),
            }

            (!errors.is_empty()).then_some(errors)
        } /*
          let mut errors = vec![];
                  let lhs = parameters.iter().map(Some).chain(std::iter::repeat(None));
                  let rhs = arguments.iter().map(Some).chain(std::iter::repeat(None));

                  // TODO: handle case where optional parameters appear before required parameters
                  for pair in lhs.zip(rhs) {
                      match pair {
                          // TODO: should this use a more nuanced approach for comparing types?
                          // how will this handle enumerators for example?
                          (Some(parameter), Some(argument))
                              if parameter.to_shape() == argument.type_of().to_shape() => {}

                          (Some(parameter), Some(argument)) => {
                              errors.push(Error::ArgumentRejected(*parameter.id(), *argument.id()));
                          }

                          (None, Some(argument)) => {
                              errors.push(Error::UnexpectedArgument(*argument.id()));
                          }

                          // TODO: this doesn't take into account default values
                          // need to bake it into the type definition
                          (Some(parameter), None) => {
                              errors.push(Error::MissingArgument(*parameter.id()));
                          }

                          (None, None) => break,
                      }
                  }

                  (!errors.is_empty()).then_some(errors)
                  */
    }
}
