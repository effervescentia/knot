use crate::parser::{
    declaration::{Declaration, DeclarationNode},
    expression::{
        ksx::{KSXNode, KSX},
        Expression, ExpressionNode,
    },
    module::{Module, ModuleNode},
    range::Range,
    types::type_expression::{TypeExpression, TypeExpressionNode},
    CharStream,
};

pub fn xr<'a>(
    x: Expression<ExpressionNode<CharStream<'a>, ()>, KSXNode<CharStream<'a>, ()>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> ExpressionNode<CharStream<'a>, ()> {
    ExpressionNode::raw(x, Range::chars(start, end))
}

pub fn kxr<'a>(
    x: KSX<ExpressionNode<CharStream<'a>, ()>, KSXNode<CharStream<'a>, ()>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> KSXNode<CharStream<'a>, ()> {
    KSXNode::raw(x, Range::chars(start, end))
}

pub fn txr<'a>(
    x: TypeExpression<TypeExpressionNode<CharStream<'a>, ()>>,
    (start, end): ((i32, i32), (i32, i32)),
) -> TypeExpressionNode<CharStream<'a>, ()> {
    TypeExpressionNode::raw(x, Range::chars(start, end))
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

pub fn mr<'a>(x: Module<DeclarationNode<CharStream<'a>, ()>>) -> ModuleNode<CharStream<'a>, ()> {
    ModuleNode::raw(x)
}