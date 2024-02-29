use std::fmt::Debug;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum RefKind {
    Type,
    Value,
    Mixed,
}

impl RefKind {
    pub fn can_accept(&self, other: &Self) -> bool {
        other == self || other == &Self::Mixed
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Enumerated<T> {
    Declaration(Vec<(String, Vec<T>)>),
    Variant(Vec<T>, T),
    Instance(T),
}

impl<T> Enumerated<T> {
    fn to_shape(&self) -> Enumerated<()> {
        match self {
            Self::Declaration(variants) => Enumerated::Declaration(
                variants
                    .iter()
                    .map(|(name, parameters)| {
                        (name.clone(), parameters.iter().map(|_| ()).collect())
                    })
                    .collect(),
            ),

            Self::Variant(parameters, _) => {
                Enumerated::Variant(parameters.iter().map(|_| ()).collect(), ())
            }

            Self::Instance(_) => Enumerated::Instance(()),
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,

    Enumerated(Enumerated<T>),
    Function(Vec<T>, T),
    View(Vec<T>),
    Module(Vec<(String, RefKind, T)>),
}

impl<T> Type<T> {
    pub fn to_shallow(&self) -> ShallowType {
        ShallowType(match self {
            Self::Nil => Type::Nil,
            Self::Boolean => Type::Boolean,
            Self::Integer => Type::Integer,
            Self::Float => Type::Float,
            Self::String => Type::String,
            Self::Style => Type::Style,
            Self::Element => Type::Element,

            Self::Enumerated(x) => Type::Enumerated(x.to_shape()),

            Self::Function(parameters, _) => {
                Type::Function(parameters.iter().map(|_| ()).collect(), ())
            }

            Self::View(parameters) => Type::View(parameters.iter().map(|_| ()).collect()),

            Self::Module(entries) => Type::Module(
                entries
                    .iter()
                    .map(|(name, kind, _)| (name.clone(), *kind, ()))
                    .collect(),
            ),
        })
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct ShallowType(pub Type<()>);

#[derive(Clone, Debug, PartialEq)]
pub struct ReferenceType<'a>(pub Type<&'a ReferenceType<'a>>);

#[derive(Clone, Debug, PartialEq)]
pub struct ConcreteType(pub Type<Box<ConcreteType>>);

impl ConcreteType {
    pub fn to_shallow(&self) -> ShallowType {
        self.0.to_shallow()
    }
}
