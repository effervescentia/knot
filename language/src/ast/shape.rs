use super::{
    declaration::{Declaration, DeclarationNode},
    expression::{Expression, ExpressionNode},
    ksx::{KSXNode, KSX},
    module::{Module, ModuleNode},
    parameter::{Parameter, ParameterNode},
    statement::{Statement, StatementNode},
    type_expression::{TypeExpression, TypeExpressionNode},
};
use crate::parser::Program;
use std::fmt::Debug;

pub trait ToShape<S> {
    fn to_shape(&self) -> S;
}

#[derive(Clone, Debug)]
pub struct ExpressionShape(pub Expression<ExpressionShape, StatementShape, KSXShape>);

impl<C> ToShape<ExpressionShape> for ExpressionNode<C> {
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

impl<C> ToShape<StatementShape> for StatementNode<C> {
    fn to_shape(&self) -> StatementShape {
        StatementShape(self.0.value().map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct KSXShape(pub KSX<ExpressionShape, KSXShape>);

impl<C> ToShape<KSXShape> for KSXNode<C> {
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

impl<C> ToShape<TypeExpressionShape> for TypeExpressionNode<C> {
    fn to_shape(&self) -> TypeExpressionShape {
        TypeExpressionShape(self.0.value().map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct ParameterShape(pub Parameter<ExpressionShape, TypeExpressionShape>);

impl<C> ToShape<ParameterShape> for ParameterNode<C> {
    fn to_shape(&self) -> ParameterShape {
        ParameterShape(self.0.value().map(&|x| x.to_shape(), &|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct DeclarationShape(
    pub Declaration<ExpressionShape, ParameterShape, ModuleShape, TypeExpressionShape>,
);

impl<C> ToShape<DeclarationShape> for DeclarationNode<C> {
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

impl<C> ToShape<ModuleShape> for ModuleNode<C> {
    fn to_shape(&self) -> ModuleShape {
        ModuleShape(self.0.map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct ProgramShape(pub ModuleShape);

impl<C> ToShape<ProgramShape> for Program<C> {
    fn to_shape(&self) -> ProgramShape {
        ProgramShape(self.0.to_shape())
    }
}
