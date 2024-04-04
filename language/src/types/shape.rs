use crate::ast;
use std::rc::Rc;

#[derive(Clone, Debug, PartialEq)]
pub struct Type(super::Type<Box<Type>>);

pub trait ToShape {
    fn to_shape(&self) -> Type;
}

impl<T> ToShape for Rc<T>
where
    T: ToShape,
{
    fn to_shape(&self) -> Type {
        self.as_ref().to_shape()
    }
}

impl<T> ToShape for super::Type<T>
where
    T: ToShape,
{
    fn to_shape(&self) -> Type {
        Type(self.map(&|x| Box::new(x.to_shape())))
    }
}

impl ToShape for ast::typed::Meta {
    fn to_shape(&self) -> Type {
        self.1.to_shape()
    }
}

impl ToShape for ast::typed::Type {
    fn to_shape(&self) -> Type {
        self.0.to_shape()
    }
}
