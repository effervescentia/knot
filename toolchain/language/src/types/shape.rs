use crate::ast;
use std::{
    fmt::{Debug, Display},
    rc::Rc,
};

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Type(pub super::Type<Box<Type>>);

impl Type {
    fn widen_object_entry(
        entry: super::ObjectTypeEntry<Box<Self>>,
    ) -> super::ObjectTypeEntry<Box<Self>> {
        match entry {
            super::ObjectTypeEntry::Optional(key, value) => {
                super::ObjectTypeEntry::Optional(key, Box::new(value.widen()))
            }
            super::ObjectTypeEntry::Required(key, value) => {
                super::ObjectTypeEntry::Required(key, Box::new(value.widen()))
            }
        }
    }

    pub fn widen(self) -> Self {
        Self(match self.0 {
            super::Type::Enumerated(name, declaration @ super::Enumerated::Declaration(_)) => {
                super::Type::Enumerated(
                    name.clone(),
                    super::Enumerated::Instance(Box::new(Self(super::Type::Enumerated(
                        name,
                        declaration,
                    )))),
                )
            }

            super::Type::Function(parameters, result) => super::Type::Function(
                parameters
                    .into_iter()
                    .map(|x| Box::new(x.widen()))
                    .collect(),
                Box::new(result.widen()),
            ),

            super::Type::Object(properties) => super::Type::Object(
                properties
                    .into_iter()
                    .map(Self::widen_object_entry)
                    .collect(),
            ),

            super::Type::View(attributes) => super::Type::View(
                attributes
                    .into_iter()
                    .map(Self::widen_object_entry)
                    .collect(),
            ),

            x => x,
        })
    }
}

impl Display for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        Display::fmt(&self.0, f)
    }
}

// TODO: try to improve this with a trait dependency like AsRef<Type>
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
