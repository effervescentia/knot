mod component;
mod declaration;
mod expression;
pub mod into_fragments;
pub mod meta;
mod module;
mod operator;
pub mod raw;
pub mod shape;
pub mod typed;
mod types;

pub use component::{Attribute, Component};
pub use declaration::{Binding, Declaration, Parameter, Storage, Visibility};
pub use expression::{Expression, Primitive, Statement};
pub use module::{Import, ImportSource, Module};
pub use operator::{BinaryOperator, UnaryOperator};
pub use types::{
    ObjectTypeExpressionEntry, TypeDeclaration, TypeExpression, TypeModule, TypePrimitive,
};

pub trait IsEmpty {
    fn is_empty(&self) -> bool;
}
