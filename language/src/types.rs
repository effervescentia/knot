use std::fmt::Debug;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Kind {
    Type,
    Value,
    Mixed,
}

impl Kind {
    pub fn can_accept(&self, other: &Self) -> bool {
        self == other || matches!((self, other), (Self::Mixed, _) | (_, Self::Mixed))
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Enumerated<T> {
    Declaration(Vec<(String, Vec<T>)>),
    Variant(Vec<T>, T),
    Instance(T),
}

impl<T> Enumerated<T> {
    pub fn map<R, F>(&self, f: &F) -> Enumerated<R>
    where
        F: Fn(&T) -> R,
    {
        match self {
            Self::Declaration(variants) => Enumerated::Declaration(
                variants
                    .iter()
                    .map(|(name, parameters)| (name.clone(), parameters.iter().map(f).collect()))
                    .collect(),
            ),

            Self::Variant(parameters, instance) => {
                Enumerated::Variant(parameters.iter().map(f).collect(), f(instance))
            }

            Self::Instance(x) => Enumerated::Instance(f(x)),
        }
    }

    pub fn to_shape(&self) -> Enumerated<()> {
        self.map(&|_| ())
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
    Module(Vec<(String, Kind, T)>),
}

impl<T> Type<T> {
    pub fn map<R, F>(&self, f: &F) -> Type<R>
    where
        F: Fn(&T) -> R,
    {
        match self {
            Self::Nil => Type::Nil,
            Self::Boolean => Type::Boolean,
            Self::Integer => Type::Integer,
            Self::Float => Type::Float,
            Self::String => Type::String,
            Self::Style => Type::Style,
            Self::Element => Type::Element,

            Self::Enumerated(x) => Type::Enumerated(x.map(f)),

            Self::Function(parameters, result) => {
                Type::Function(parameters.iter().map(f).collect(), f(result))
            }

            Self::View(parameters) => Type::View(parameters.iter().map(f).collect()),

            Self::Module(declarations) => Type::Module(
                declarations
                    .iter()
                    .map(|(name, kind, x)| (name.clone(), *kind, f(x)))
                    .collect(),
            ),
        }
    }

    pub fn to_shape(&self) -> Type<()> {
        self.map(&|_| ())
    }
}
