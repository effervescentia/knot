use super::ScopeContext;
use crate::{
    analyzer::{
        context::NodeContext,
        register::{Identify, Register},
    },
    parser::{
        declaration::{self, parameter::Parameter, Declaration},
        expression::ExpressionNode,
        position::Decrement,
        types::type_expression::TypeExpressionNode,
    },
};
use combine::Stream;
use std::fmt::Debug;

fn identify_parameters<T>(
    xs: Vec<Parameter<ExpressionNode<T, ()>, TypeExpressionNode<T, ()>>>,
    ctx: &mut ScopeContext,
) -> Vec<Parameter<ExpressionNode<T, NodeContext>, TypeExpressionNode<T, NodeContext>>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter()
        .map(
            |Parameter {
                 name,
                 value_type,
                 default_value,
             }| Parameter {
                name,
                value_type: value_type.map(|x| x.register(ctx)),
                default_value: default_value.map(|x| x.register(ctx)),
            },
        )
        .collect::<Vec<_>>()
}

impl<T> Identify<declaration::NodeValue<T, NodeContext>> for declaration::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(self, ctx: &mut ScopeContext) -> declaration::NodeValue<T, NodeContext> {
        match self {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name,
                value: value.register(ctx),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name,
                variants: variants
                    .into_iter()
                    .map(|(name, xs)| {
                        (
                            name,
                            xs.into_iter().map(|x| x.register(ctx)).collect::<Vec<_>>(),
                        )
                    })
                    .collect::<Vec<_>>(),
            },

            Declaration::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name,
                value_type: value_type.map(|x| x.register(ctx)),
                value: value.register(ctx),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name,
                parameters: identify_parameters(parameters, ctx),
                body_type: body_type.map(|x| x.register(ctx)),
                body: body.register(ctx),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name,
                parameters: identify_parameters(parameters, ctx),
                body: body.register(ctx),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name,
                value: value.register(ctx),
            },
        }
    }
}
