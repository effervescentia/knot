use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        declaration::{self, parameter::Parameter, Declaration},
        expression::ExpressionNode,
        position::Decrement,
        types::type_expression::TypeExpressionNode,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for declaration::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        let parameters_to_refs = |xs: &Vec<
            Parameter<ExpressionNode<T, NodeContext>, TypeExpressionNode<T, NodeContext>>,
        >| {
            xs.into_iter()
                .map(
                    |Parameter {
                         name,
                         value_type,
                         default_value,
                     }| Parameter {
                        name: name.clone(),
                        value_type: value_type.as_ref().map(|x| *x.node().id()),
                        default_value: default_value.as_ref().map(|x| *x.node().id()),
                    },
                )
                .collect::<Vec<_>>()
        };

        Fragment::Declaration(match self {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name: name.clone(),
                value: *value.0.id(),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name: name.clone(),
                variants: variants
                    .into_iter()
                    .map(|(name, params)| {
                        (
                            name.clone(),
                            params
                                .into_iter()
                                .map(|x| *x.node().id())
                                .collect::<Vec<_>>(),
                        )
                    })
                    .collect::<Vec<_>>(),
            },

            Declaration::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name: name.clone(),
                value_type: value_type.as_ref().map(|x| *x.node().id()),
                value: *value.0.id(),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name: name.clone(),
                parameters: parameters_to_refs(parameters),
                body_type: body_type.as_ref().map(|x| *x.node().id()),
                body: *body.node().id(),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name: name.clone(),
                parameters: parameters_to_refs(parameters),
                body: *body.node().id(),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name: name.clone(),
                value: *value.id(),
            },
        })
    }
}
