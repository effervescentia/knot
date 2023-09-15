use super::ScopeContext;
use crate::{
    analyzer::{
        context::NodeContext,
        register::{Identify, Register},
    },
    parser::{
        expression::ExpressionNode,
        ksx::{self, KSXNode, KSX},
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

fn identify_attributes<T>(
    xs: Vec<(String, Option<ExpressionNode<T, ()>>)>,
    ctx: &mut ScopeContext,
) -> Vec<(String, Option<ExpressionNode<T, NodeContext>>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter()
        .map(|(key, value)| (key, value.map(|x| x.register(ctx))))
        .collect::<Vec<_>>()
}

fn identify_children<T>(
    xs: Vec<KSXNode<T, ()>>,
    ctx: &mut ScopeContext,
) -> Vec<KSXNode<T, NodeContext>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter().map(|x| x.register(ctx)).collect::<Vec<_>>()
}

impl<T> Identify<ksx::NodeValue<T, NodeContext>> for ksx::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(self, ctx: &mut ScopeContext) -> ksx::NodeValue<T, NodeContext> {
        match self {
            KSX::Text(x) => KSX::Text(x),

            KSX::Inline(x) => KSX::Inline(x.register(ctx)),

            KSX::Fragment(children) => KSX::Fragment(identify_children(children, ctx)),

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag, identify_attributes(attributes, ctx))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag,
                identify_attributes(attributes, ctx),
                identify_children(children, ctx),
                end_tag,
            ),
        }
    }
}
