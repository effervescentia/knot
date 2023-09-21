use crate::{
    analyzer::{context::NodeContext, infer::strong::ToStrong, Strong, Type},
    ast::ksx::KSXNode,
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
        KSXNode(Node(
            self.node()
                .value()
                .map(&|x| x.to_strong(), &|x| x.to_strong()),
            self.node().range().clone(),
            Strong::Type(Type::Nil),
        ))
    }
}
