use crate::parser::{
    declaration::{Declaration, DeclarationNode},
    expression::{
        ksx::{KSXNode, KSX},
        Expression, ExpressionNode,
    },
    module::{Module, ModuleNode},
    node::Node,
    range::Range,
    types::type_expression::{TypeExpression, TypeExpressionNode},
    CharStream,
};

const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

pub fn xr<'a>(
    x: Expression<ExpressionNode<CharStream<'a>, ()>, KSXNode<CharStream<'a>, ()>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> ExpressionNode<CharStream<'a>, ()> {
    ExpressionNode::raw(x, Range::chars(start, end))
}

pub fn xc<T>(
    x: Expression<ExpressionNode<CharStream<'static>, T>, KSXNode<CharStream<'static>, T>>,
    ctx: T,
) -> ExpressionNode<CharStream<'static>, T> {
    ExpressionNode(Node::new(x, RANGE, ctx))
}

pub fn kxr<'a>(
    x: KSX<ExpressionNode<CharStream<'a>, ()>, KSXNode<CharStream<'a>, ()>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> KSXNode<CharStream<'a>, ()> {
    KSXNode::raw(x, Range::chars(start, end))
}

pub fn kxc<T>(
    x: KSX<ExpressionNode<CharStream<'static>, T>, KSXNode<CharStream<'static>, T>>,
    ctx: T,
) -> KSXNode<CharStream<'static>, T> {
    KSXNode(Node::new(x, RANGE, ctx))
}

pub fn txr<'a>(
    x: TypeExpression<TypeExpressionNode<CharStream<'a>, ()>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> TypeExpressionNode<CharStream<'a>, ()> {
    TypeExpressionNode::raw(x, Range::chars(start, end))
}

pub fn txc<T>(
    x: TypeExpression<TypeExpressionNode<CharStream<'static>, T>>,
    ctx: T,
) -> TypeExpressionNode<CharStream<'static>, T> {
    TypeExpressionNode(Node::new(x, RANGE, ctx))
}

pub fn dr<'a>(
    x: Declaration<
        ExpressionNode<CharStream<'a>, ()>,
        ModuleNode<CharStream<'a>, ()>,
        TypeExpressionNode<CharStream<'a>, ()>,
    >,
    (start, end): ((i32, i32), (i32, i32)),
) -> DeclarationNode<CharStream<'a>, ()> {
    DeclarationNode::raw(x, Range::chars(start, end))
}

pub fn dc<T>(
    x: Declaration<
        ExpressionNode<CharStream<'static>, T>,
        ModuleNode<CharStream<'static>, T>,
        TypeExpressionNode<CharStream<'static>, T>,
    >,
    ctx: T,
) -> DeclarationNode<CharStream<'static>, T> {
    DeclarationNode(Node::new(x, RANGE, ctx))
}

pub fn mr<'a>(x: Module<DeclarationNode<CharStream<'a>, ()>>) -> ModuleNode<CharStream<'a>, ()> {
    ModuleNode::raw(x)
}
