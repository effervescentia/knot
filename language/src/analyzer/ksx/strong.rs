use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::ksx::{KSXNode, KSX},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ToStrong<KSXNode<T, Strong>> for KSXNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self) -> KSXNode<T, Strong> {
        let node = self.node();
        KSXNode(Node(
            match node.0 {
                KSX::Text(x) => KSX::Text(x),

                KSX::Inline(x) => KSX::Inline(x.to_strong()),

                KSX::Fragment(xs) => {
                    KSX::Fragment(xs.into_iter().map(|x| x.to_strong()).collect::<Vec<_>>())
                }

                KSX::ClosedElement(tag, xs) => KSX::ClosedElement(
                    tag,
                    xs.into_iter()
                        .map(|(key, value)| (key, value.map(|x| x.to_strong())))
                        .collect::<Vec<_>>(),
                ),

                KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                    start_tag,
                    attributes
                        .into_iter()
                        .map(|(key, value)| (key, value.map(|x| x.to_strong())))
                        .collect::<Vec<_>>(),
                    children
                        .into_iter()
                        .map(|x| x.to_strong())
                        .collect::<Vec<_>>(),
                    end_tag,
                ),
            },
            node.1.clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
