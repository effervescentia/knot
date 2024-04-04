pub use super::meta::Binding;
use crate::{types, CanonicalId};
use std::rc::Rc;

pub type Meta = (CanonicalId, Type);

#[derive(Clone, Debug, PartialEq)]
pub struct Type(pub types::Type<Rc<(CanonicalId, Type)>>);

impl Type {
    pub fn to_canonical(&self) -> types::Type<CanonicalId> {
        self.0.map(&|x| x.0)
    }
}

pub type Storage = super::Storage<Binding>;
pub type Expression = super::meta::Expression<Meta>;
pub type Statement = super::meta::Statement<Meta>;
pub type Component = super::meta::Component<Meta>;
pub type TypeExpression = super::meta::TypeExpression<Meta>;
pub type Parameter = super::meta::Parameter<Meta>;
pub type Declaration = super::meta::Declaration<Meta>;
pub type Import = super::meta::Import<Meta>;
pub type Module = super::meta::Module<Meta>;
pub type Program = super::meta::Program<Meta>;
