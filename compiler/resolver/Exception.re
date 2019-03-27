exception ExecutingNonFunction;
exception OperatorTypeMismatch;
exception DefaultValueTypeMismatch;
exception InvalidTypeReference;
exception TypeNotSet;
exception NameInUse(string);
exception UsedBeforeDeclaration(string);
exception PropLookupFailed;
exception ResultLookupFailed;
exception UnanalyzedTypeReference;
exception GenericConstant;
exception InferredModuleType;
exception InvalidImport;
exception ImportedModuleDoesNotExist;
exception DeclarationNotFound(string);

exception TypeResolutionNotSupported;
