use crate::Range;
pub use lang::test::fixture::a;

#[derive(Clone, Copy, PartialEq, Eq)]
pub struct MockRange;

#[allow(dead_code)]
const RANGE: Range = Range((1, 1), (1, 1));

#[allow(dead_code)]
type InitRange = ((i32, i32), (i32, i32));

/// node factories
#[allow(dead_code)]
pub mod n {
    use super::*;
    pub use lang::test::fixture::n::*;
    use lang::{
        ast::{
            DeclarationNode, DeclarationNodeValue, ExpressionNode, ExpressionNodeValue, ImportNode,
            ImportNodeValue, KSXNode, KSXNodeValue, ModuleNode, ModuleNodeValue, ParameterNode,
            ParameterNodeValue, StatementNode, StatementNodeValue, TypeExpressionNode,
            TypeExpressionNodeValue,
        },
        Node,
    };

    pub const fn x(x: ExpressionNodeValue<Range, ()>) -> ExpressionNode<Range, ()> {
        ExpressionNode::raw(x, RANGE)
    }

    pub const fn xr(
        x: ExpressionNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> ExpressionNode<Range, ()> {
        ExpressionNode::raw(x, Range(start, end))
    }

    pub const fn xc<T>(x: ExpressionNodeValue<Range, T>, ctx: T) -> ExpressionNode<Range, T> {
        ExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub const fn s(x: StatementNodeValue<Range, ()>) -> StatementNode<Range, ()> {
        StatementNode::raw(x, RANGE)
    }

    pub const fn sr(
        x: StatementNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> StatementNode<Range, ()> {
        StatementNode::raw(x, Range(start, end))
    }

    pub const fn sc<T>(x: StatementNodeValue<Range, T>, ctx: T) -> StatementNode<Range, T> {
        StatementNode(Node::new(x, RANGE, ctx))
    }

    pub const fn kx(x: KSXNodeValue<Range, ()>) -> KSXNode<Range, ()> {
        KSXNode::raw(x, RANGE)
    }

    pub const fn kxr(x: KSXNodeValue<Range, ()>, (start, end): InitRange) -> KSXNode<Range, ()> {
        KSXNode::raw(x, Range(start, end))
    }

    pub const fn kxc<T>(x: KSXNodeValue<Range, T>, ctx: T) -> KSXNode<Range, T> {
        KSXNode(Node::new(x, RANGE, ctx))
    }

    pub const fn p(x: ParameterNodeValue<Range, ()>) -> ParameterNode<Range, ()> {
        ParameterNode::raw(x, RANGE)
    }

    pub const fn pr(
        x: ParameterNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> ParameterNode<Range, ()> {
        ParameterNode::raw(x, Range(start, end))
    }

    pub const fn pc<T>(x: ParameterNodeValue<Range, T>, ctx: T) -> ParameterNode<Range, T> {
        ParameterNode(Node::new(x, RANGE, ctx))
    }

    pub const fn tx(x: TypeExpressionNodeValue<Range, ()>) -> TypeExpressionNode<Range, ()> {
        TypeExpressionNode::raw(x, RANGE)
    }

    pub const fn txr(
        x: TypeExpressionNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> TypeExpressionNode<Range, ()> {
        TypeExpressionNode::raw(x, Range(start, end))
    }

    pub const fn txc<T>(
        x: TypeExpressionNodeValue<Range, T>,
        ctx: T,
    ) -> TypeExpressionNode<Range, T> {
        TypeExpressionNode(Node::new(x, RANGE, ctx))
    }

    pub const fn d(x: DeclarationNodeValue<Range, ()>) -> DeclarationNode<Range, ()> {
        DeclarationNode::raw(x, RANGE)
    }

    pub const fn dr(
        x: DeclarationNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> DeclarationNode<Range, ()> {
        DeclarationNode::raw(x, Range(start, end))
    }

    pub const fn dc<T>(x: DeclarationNodeValue<Range, T>, ctx: T) -> DeclarationNode<Range, T> {
        DeclarationNode(Node::new(x, RANGE, ctx))
    }

    pub const fn i(x: ImportNodeValue) -> ImportNode<Range, ()> {
        ImportNode::raw(x, RANGE)
    }

    pub const fn ir(x: ImportNodeValue, (start, end): InitRange) -> ImportNode<Range, ()> {
        ImportNode::raw(x, Range(start, end))
    }

    pub const fn ic<T>(x: ImportNodeValue, ctx: T) -> ImportNode<Range, T> {
        ImportNode(Node::new(x, RANGE, ctx))
    }

    pub const fn m(x: ModuleNodeValue<Range, ()>) -> ModuleNode<Range, ()> {
        ModuleNode::raw(x)
    }

    pub const fn mr<R>(x: ModuleNodeValue<R, ()>) -> ModuleNode<R, ()> {
        ModuleNode::raw(x)
    }
}
