pub use super::meta::Binding;
use crate::{types, CanonicalId};
use std::rc::Rc;

#[derive(Clone, Debug, PartialEq)]
pub struct Type(pub types::Type<Rc<(CanonicalId, Type)>>);

impl Type {
    pub fn to_canonical(&self) -> types::Type<CanonicalId> {
        self.0.map(&|x| x.0)
    }
}

pub type Storage = super::Storage<Binding>;
pub type Expression = super::meta::Expression<Type>;
pub type Statement = super::meta::Statement<Type>;
pub type Component = super::meta::Component<Type>;
pub type TypeExpression = super::meta::TypeExpression<Type>;
pub type Parameter = super::meta::Parameter<Type>;
pub type Declaration = super::meta::Declaration<Type>;
pub type Import = super::meta::Import<Type>;
pub type Module = super::meta::Module<Type>;
pub type Program = super::meta::Program<Type>;
