use crate::{
    analyzer::{context::NodeContext, fragment::Fragment, register::ToFragment},
    parser::{
        expression::{
            ksx::{self, KSX},
            ExpressionNode,
        },
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToFragment for ksx::NodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        let attributes_to_refs = |xs: &Vec<(String, Option<ExpressionNode<T, NodeContext>>)>| {
            xs.into_iter()
                .map(|(key, value)| (key.clone(), value.as_ref().map(|x| *x.node().id())))
                .collect::<Vec<_>>()
        };

        Fragment::KSX(match self {
            KSX::Text(x) => KSX::Text(x.clone()),

            KSX::Inline(x) => KSX::Inline(*x.0.id()),

            KSX::Fragment(xs) => {
                KSX::Fragment(xs.into_iter().map(|x| *x.0.id()).collect::<Vec<_>>())
            }

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag.clone(), attributes_to_refs(attributes))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag.clone(),
                attributes_to_refs(attributes),
                children.into_iter().map(|x| *x.0.id()).collect::<Vec<_>>(),
                end_tag.clone(),
            ),
        })
    }
}
