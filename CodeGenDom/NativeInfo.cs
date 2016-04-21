using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

using Sce.Atf.Dom;

namespace DomGen
{
    // Strings found in the .xsd file and those added as tags to the DOM objects.
    public static class SchemaStrings
    {
        public const string LegeNativeType = "LeGe.NativeType";
        public const string LeGeNativeProperty = "LeGe.NativeProperty";
        public const string LeGeNativeElement = "LeGe.NativeElement";
        public const string NativeName = "nativeName";
        public const string NativeType = "nativeType";        
        public const string Name = "name";
        public const string Access = "access";
        public const string Set = "set";
        public const string Get = "get";
    }

    // A native list is an attribute of a class that has Add/Remove functionality.
    public class NativeListInfo
    {
        public NativeListInfo(XmlElement elm)
        {
            NativeName = elm.GetAttribute(SchemaStrings.NativeName);
            NativeType = elm.GetAttribute(SchemaStrings.NativeType);
            Access = elm.GetAttribute(SchemaStrings.Access);

            bool canSet = true; 
            bool canGet = true;
            if (!String.IsNullOrEmpty(Access))
            {
                canSet = Access.Contains(SchemaStrings.Set);
                canGet = Access.Contains(SchemaStrings.Get);
            }
            Setable = canSet;
            Getable = canGet;
        }
        public string NativeName
        {
            get;
            private set;
        }
        public string NativeType
        {
            get;
            private set;
        }
        public string Access
        {
            get;
            private set;
        }
        public bool Setable
        {
            get;
            private set;
        }
        public bool Getable
        {
            get;
            private set;
        }
    }

    // A native property is an attribute of a class that has get/set functions.
    public class NativePropertyInfo
    {
        public NativePropertyInfo(XmlElement elm)
        {
            NativeName = elm.GetAttribute(SchemaStrings.NativeName);
            NativeType = elm.GetAttribute(SchemaStrings.NativeType);
            Access = elm.GetAttribute(SchemaStrings.Access);

            bool canSet = true;
            bool canGet = true;
            if (!String.IsNullOrEmpty(Access))
            {
                canSet = Access.Contains(SchemaStrings.Set);
                canGet = Access.Contains(SchemaStrings.Get);
            }
            Setable = canSet;
            Getable = canGet;

        }
        public string NativeName
        {
            get;
            private set;
        }
        public string NativeType
        {
            get;
            private set;
        }
        public string Access
        {
            get;
            private set;
        }
        public bool Setable
        {
            get;
            private set;
        }
        public bool Getable
        {
            get;
            private set;
        }

    }

    // A native class in a class that can be instantiated by the native code.
    // The class info stores the name of the class as well as information 
    // about all the native properties and lists.
    public class NativeClassInfo
    {
        public NativeClassInfo(XmlElement element, bool abstractType)
        {
            NativeName = element.GetAttribute(SchemaStrings.NativeName);
            Abstract = abstractType;
            m_properties = new List<NativePropertyInfo>();
            m_lists = new List<NativeListInfo>();
        }
        
        public string NativeName
        {
            get;
            private set;
        }
        public bool Abstract
        {
            get;
            private set;
        }


        private List<NativePropertyInfo> m_properties;
        public List<NativePropertyInfo> Properties
        {
            get { return m_properties; }
        }

        private List<NativeListInfo> m_lists;
        public List<NativeListInfo> Lists
        {
            get { return m_lists; }
        }

    }

    // The native schema info contains all the information about 
    // classes, properties and list supported by the native code.
    public class NativeSchemaInfo
    {
        public NativeSchemaInfo(XmlSchemaTypeLoader typeLoader)
        {
            m_nativeClasses = new List<NativeClassInfo>();

            // parse schema & add our Annotations
            foreach (DomNodeType domType in typeLoader.GetNodeTypes())
            {
                IEnumerable<XmlNode> annotations = domType.GetTagLocal<IEnumerable<XmlNode>>();
                if (annotations == null)
                    continue;
                                
                NativeClassInfo classInfo = null;
                foreach (XmlNode annot in annotations)
                {
                    XmlElement elm = annot as XmlElement;
                    if (elm.LocalName == SchemaStrings.LegeNativeType)
                    {
                        classInfo = new NativeClassInfo(elm, domType.IsAbstract);
                        m_nativeClasses.Add(classInfo);
                        break;
                    }
                }

                if (classInfo == null) continue;

                foreach (XmlNode annot in annotations)
                {
                    XmlElement elm = annot as XmlElement;
                    if (elm.LocalName == SchemaStrings.LeGeNativeProperty)
                    {
                        NativePropertyInfo info = new NativePropertyInfo(elm);
                        classInfo.Properties.Add(info);
                    }
                    else if (elm.LocalName == SchemaStrings.LeGeNativeElement)
                    {
                        NativeListInfo info = new NativeListInfo(elm);
                        classInfo.Lists.Add(info);
                    }
                }
            }
        }

        private List<NativeClassInfo> m_nativeClasses;
        public List<NativeClassInfo> NativeClasses
        {
            get { return m_nativeClasses; }
        }

    }
}