use super::{DeclarationNode, ImportNode};
use crate::Identity;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Module<I, D> {
    pub imports: Vec<I>,
    pub declarations: Vec<D>,
}

impl<I, D> Module<I, D> {
    pub fn new(imports: Vec<I>, declarations: Vec<D>) -> Self {
        Self {
            imports,
            declarations,
        }
    }

    pub fn map<I2, D2>(&self, fi: &impl Fn(&I) -> I2, fd: &impl Fn(&D) -> D2) -> Module<I2, D2> {
        Module {
            imports: self.imports.iter().map(fi).collect(),
            declarations: self.declarations.iter().map(fd).collect(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.imports.is_empty() || self.declarations.is_empty()
    }
}

pub type ModuleNodeValue<R, C> = Module<ImportNode<R, C>, DeclarationNode<R, C>>;

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
