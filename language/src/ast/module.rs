use super::{DeclarationNode, Import};
use crate::Identity;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Module<D> {
    pub imports: Vec<Import>,
    pub declarations: Vec<D>,
}

impl<D> Module<D> {
    pub fn new(imports: Vec<Import>, declarations: Vec<D>) -> Self {
        Self {
            imports,
            declarations,
        }
    }

    pub fn map<D2>(&self, fd: &impl Fn(&D) -> D2) -> Module<D2> {
        Module {
            imports: self.imports.clone(),
            declarations: self.declarations.iter().map(fd).collect(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.imports.is_empty() || self.declarations.is_empty()
    }
}

pub type ModuleNodeValue<R, C> = Module<DeclarationNode<R, C>>;

#[derive(Debug, PartialEq)]
pub struct ModuleNode<R, C>(pub ModuleNodeValue<R, C>, pub C);

impl<R, C> ModuleNode<R, C> {
    pub fn map<C2>(
        &self,
        f: impl Fn(&ModuleNodeValue<R, C>, &C) -> (ModuleNodeValue<R, C2>, C2),
    ) -> ModuleNode<R, C2> {
        let (value, ctx) = f(&self.0, &self.1);

        ModuleNode(value, ctx)
    }
}

impl<R> ModuleNode<R, ()> {
    pub fn raw(x: ModuleNodeValue<R, ()>) -> Self {
        Self(x, ())
    }
}

impl<R, T> ModuleNode<R, T>
where
    T: Identity<usize>,
{
    pub fn id(&self) -> &usize {
        self.1.id()
    }
}
