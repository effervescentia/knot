use super::{
    Declaration, DeclarationNode, Expression, ExpressionNode, Import, ImportNode, KSXNode, Module,
    ModuleNode, Parameter, ParameterNode, Statement, StatementNode, TypeExpression,
    TypeExpressionNode, KSX,
};
use crate::Program;
use std::fmt::Debug;

pub trait ToShape<S> {
    fn to_shape(&self) -> S;
}

#[derive(Clone, Debug)]
pub struct ExpressionShape(pub Expression<ExpressionShape, StatementShape, KSXShape>);

impl<R, C> ToShape<ExpressionShape> for ExpressionNode<R, C> {
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

impl<R, C> ToShape<StatementShape> for StatementNode<R, C> {
    fn to_shape(&self) -> StatementShape {
        StatementShape(self.0.value().map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct KSXShape(pub KSX<ExpressionShape, KSXShape>);

impl<R, C> ToShape<KSXShape> for KSXNode<R, C> {
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

impl<R, C> ToShape<TypeExpressionShape> for TypeExpressionNode<R, C> {
    fn to_shape(&self) -> TypeExpressionShape {
        TypeExpressionShape(self.0.value().map(&|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct ParameterShape(pub Parameter<ExpressionShape, TypeExpressionShape>);

impl<R, C> ToShape<ParameterShape> for ParameterNode<R, C> {
    fn to_shape(&self) -> ParameterShape {
        ParameterShape(self.0.value().map(&|x| x.to_shape(), &|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct DeclarationShape(
    pub Declaration<ExpressionShape, ParameterShape, ModuleShape, TypeExpressionShape>,
);

impl<R, C> ToShape<DeclarationShape> for DeclarationNode<R, C> {
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
pub struct ImportShape(pub Import);

impl<R, C> ToShape<ImportShape> for ImportNode<R, C> {
    fn to_shape(&self) -> ImportShape {
        ImportShape(self.0.value().clone())
    }
}

#[derive(Clone, Debug)]
pub struct ModuleShape(pub Module<ImportShape, DeclarationShape>);

impl<R, C> ToShape<ModuleShape> for ModuleNode<R, C> {
    fn to_shape(&self) -> ModuleShape {
        ModuleShape(self.0.map(&|x| x.to_shape(), &|x| x.to_shape()))
    }
}

#[derive(Clone, Debug)]
pub struct ProgramShape(pub ModuleShape);

impl<R, C> ToShape<ProgramShape> for Program<R, C> {
    fn to_shape(&self) -> ProgramShape {
        ProgramShape(self.0.to_shape())
    }
}
