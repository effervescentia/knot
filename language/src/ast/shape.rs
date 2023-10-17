use super::{
    declaration::{Declaration, DeclarationNode},
    expression::{Expression, ExpressionNode},
    ksx::{KSXNode, KSX},
    module::{Module, ModuleNode},
    parameter::{Parameter, ParameterNode},
    statement::{Statement, StatementNode},
    type_expression::{TypeExpression, TypeExpressionNode},
};
use crate::{common::position::Decrement, parser::Program};
use combine::Stream;
use std::fmt::Debug;

pub trait ToShape<S> {
    fn to_shape(&self) -> S;
}

#[derive(Clone, Debug)]
pub struct ExpressionShape(pub Expression<ExpressionShape, StatementShape, KSXShape>);

impl<T, C> ToShape<ExpressionShape> for ExpressionNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> ExpressionShape {
        ExpressionShape(self.0.value().map(
            &mut |x| x.to_shape(),
            &mut |x| x.to_shape(),
            &mut |x| x.to_shape(),
        ))
    }
}

#[derive(Clone, Debug)]
pub struct StatementShape(pub Statement<ExpressionShape>);

impl<T, C> ToShape<StatementShape> for StatementNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> StatementShape {
        StatementShape(self.0.value().map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct KSXShape(pub KSX<ExpressionShape, KSXShape>);

impl<T, C> ToShape<KSXShape> for KSXNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> KSXShape {
        KSXShape(
            self.0
                .value()
                .map(&mut |x| x.to_shape(), &mut |x| x.to_shape()),
        )
    }
}

#[derive(Clone, Debug)]
pub struct TypeExpressionShape(pub TypeExpression<TypeExpressionShape>);

impl<T, C> ToShape<TypeExpressionShape> for TypeExpressionNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> TypeExpressionShape {
        TypeExpressionShape(self.0.value().map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct ParameterShape(pub Parameter<ExpressionShape, TypeExpressionShape>);

impl<T, C> ToShape<ParameterShape> for ParameterNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> ParameterShape {
        ParameterShape(self.0.value().map(&|x| x.to_shape(), &|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct DeclarationShape(
    pub Declaration<ExpressionShape, ParameterShape, ModuleShape, TypeExpressionShape>,
);

impl<T, C> ToShape<DeclarationShape> for DeclarationNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> DeclarationShape {
        DeclarationShape(self.0.value().map(
            &|x| x.to_shape(),
            &|x| x.to_shape(),
            &|x| x.to_shape(),
            &|x| x.to_shape(),
        ))
    }
}

#[derive(Clone, Debug)]
pub struct ModuleShape(pub Module<DeclarationShape>);

impl<T, C> ToShape<ModuleShape> for ModuleNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_shape(&self) -> ModuleShape {
        ModuleShape(self.0.map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct ProgramShape(pub ModuleShape);

impl<'a, C> ToShape<ProgramShape> for Program<'a, C> {
    fn to_shape(&self) -> ProgramShape {
        ProgramShape(self.0.to_shape())
    }
}
