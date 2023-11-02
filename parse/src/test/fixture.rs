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
    use lang::ast::{
        AstNode, DeclarationNode, DeclarationNodeValue, ExpressionNode, ExpressionNodeValue,
        ImportNode, ImportNodeValue, KSXNode, KSXNodeValue, ModuleNode, ModuleNodeValue,
        ParameterNode, ParameterNodeValue, StatementNode, StatementNodeValue, TypeExpressionNode,
        TypeExpressionNodeValue,
    };
    pub use lang::test::fixture::n::*;

    pub fn x(x: ExpressionNodeValue<Range, ()>) -> ExpressionNode<Range, ()> {
        ExpressionNode::raw(x, RANGE)
    }

    pub fn xr(
        x: ExpressionNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> ExpressionNode<Range, ()> {
        ExpressionNode::raw(x, Range(start, end))
    }

    pub fn xc<T>(x: ExpressionNodeValue<Range, T>, ctx: T) -> ExpressionNode<Range, T> {
        ExpressionNode::new(x, RANGE, ctx)
    }

    pub fn s(x: StatementNodeValue<Range, ()>) -> StatementNode<Range, ()> {
        StatementNode::raw(x, RANGE)
    }

    pub fn sr(
        x: StatementNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> StatementNode<Range, ()> {
        StatementNode::raw(x, Range(start, end))
    }

    pub fn sc<T>(x: StatementNodeValue<Range, T>, ctx: T) -> StatementNode<Range, T> {
        StatementNode::new(x, RANGE, ctx)
    }

    pub fn kx(x: KSXNodeValue<Range, ()>) -> KSXNode<Range, ()> {
        KSXNode::raw(x, RANGE)
    }

    pub fn kxr(x: KSXNodeValue<Range, ()>, (start, end): InitRange) -> KSXNode<Range, ()> {
        KSXNode::raw(x, Range(start, end))
    }

    pub fn kxc<T>(x: KSXNodeValue<Range, T>, ctx: T) -> KSXNode<Range, T> {
        KSXNode::new(x, RANGE, ctx)
    }

    pub fn p(x: ParameterNodeValue<Range, ()>) -> ParameterNode<Range, ()> {
        ParameterNode::raw(x, RANGE)
    }

    pub fn pr(
        x: ParameterNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> ParameterNode<Range, ()> {
        ParameterNode::raw(x, Range(start, end))
    }

    pub fn pc<T>(x: ParameterNodeValue<Range, T>, ctx: T) -> ParameterNode<Range, T> {
        ParameterNode::new(x, RANGE, ctx)
    }

    pub fn tx(x: TypeExpressionNodeValue<Range, ()>) -> TypeExpressionNode<Range, ()> {
        TypeExpressionNode::raw(x, RANGE)
    }

    pub fn txr(
        x: TypeExpressionNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> TypeExpressionNode<Range, ()> {
        TypeExpressionNode::raw(x, Range(start, end))
    }

    pub fn txc<T>(x: TypeExpressionNodeValue<Range, T>, ctx: T) -> TypeExpressionNode<Range, T> {
        TypeExpressionNode::new(x, RANGE, ctx)
    }

    pub fn d(x: DeclarationNodeValue<Range, ()>) -> DeclarationNode<Range, ()> {
        DeclarationNode::raw(x, RANGE)
    }

    pub fn dr(
        x: DeclarationNodeValue<Range, ()>,
        (start, end): InitRange,
    ) -> DeclarationNode<Range, ()> {
        DeclarationNode::raw(x, Range(start, end))
    }

    pub fn dc<T>(x: DeclarationNodeValue<Range, T>, ctx: T) -> DeclarationNode<Range, T> {
        DeclarationNode::new(x, RANGE, ctx)
    }

    pub fn i(x: ImportNodeValue) -> ImportNode<Range, ()> {
        ImportNode::<Range, ()>::raw(x, RANGE)
    }

    pub fn ir(x: ImportNodeValue, (start, end): InitRange) -> ImportNode<Range, ()> {
        ImportNode::<Range, ()>::raw(x, Range(start, end))
    }

    pub fn ic<T>(x: ImportNodeValue, ctx: T) -> ImportNode<Range, T> {
        ImportNode::<Range, T>::new(x, RANGE, ctx)
    }

    pub const fn m(x: ModuleNodeValue<Range, ()>) -> ModuleNode<Range, ()> {
        ModuleNode::raw(x)
    }

    pub const fn mr<R>(x: ModuleNodeValue<R, ()>) -> ModuleNode<R, ()> {
        ModuleNode::raw(x)
    }
}
