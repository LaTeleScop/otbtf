/*=========================================================================

  Copyright (c) 2018-2019 Remi Cresson (IRSTEA)
  Copyright (c) 2020-2021 Remi Cresson (INRAE)


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef otbTensorflowMultisourceModelBase_h
#define otbTensorflowMultisourceModelBase_h

#include "itkProcessObject.h"
#include "itkNumericTraits.h"
#include "itkSimpleDataObjectDecorator.h"

// Tensorflow
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/env.h"

// Tensorflow helpers
#include "otbTensorflowGraphOperations.h"
#include "otbTensorflowDataTypeBridge.h"
#include "otbTensorflowCopyUtils.h"
#include "otbTensorflowCommon.h"

namespace otb
{

/**
 * \class TensorflowMultisourceModelBase
 * \brief This filter is the base class for all TensorFlow model filters.
 *
 * This abstract class implements a number of generic methods that are used in
 * filters that use the TensorFlow engine.
 *
 * The filter has N input images (Input), each one corresponding to a placeholder
 * that will fed the TensorFlow model. For each input, the name of the
 * placeholder (InputPlaceholders, a std::vector of std::string) and the
 * receptive field (InputReceptiveFields, a std::vector of SizeType) i.e. the
 * input space that the model will "see", must be provided. Hence the number of
 * input images, and the size of InputPlaceholders and InputReceptiveFields must
 * be the same. If not, an exception will be thrown during the method
 * GenerateOutputInformation().
 *
 * The TensorFlow graph and session must be set using the SetGraph() and
 * SetSession() methods.
 *
 * Target nodes names of the TensorFlow graph that must be triggered can be set
 * with the SetTargetNodesNames.
 *
 * The OutputTensorNames consists in a std::vector of std::string, and
 * corresponds to the names of tensors that will be computed during the session.
 * As for input placeholders, output tensors field of expression
 * (OutputExpressionFields, a std::vector of SizeType), i.e. the output
 * space that the TensorFlow model will "generate", must be provided.
 *
 * Finally, a list of scalar placeholders can be fed in the form of std::vector
 * of std::string, each one expressing the assignment of a single valued
 * placeholder, e.g. "drop_rate=0.5 learning_rate=0.002 toto=true".
 * See otb::tf::ExpressionToTensor() to know more about syntax.
 *
 * \ingroup OTBTensorflow
 */
template <class TInputImage, class TOutputImage=TInputImage>
class ITK_EXPORT TensorflowMultisourceModelBase :
public itk::ImageToImageFilter<TInputImage, TOutputImage>
{

public:

  /** Standard class typedefs. */
  typedef TensorflowMultisourceModelBase             Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>                    Pointer;
  typedef itk::SmartPointer<const Self>              ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(TensorflowMultisourceModelBase, itk::ImageToImageFilter);

  /** Images typedefs */
  typedef TInputImage                                ImageType;
  typedef typename TInputImage::Pointer              ImagePointerType;
  typedef typename TInputImage::PixelType            PixelType;
  typedef typename TInputImage::InternalPixelType    InternalPixelType;
  typedef typename TInputImage::IndexType            IndexType;
  typedef typename TInputImage::IndexValueType       IndexValueType;
  typedef typename TInputImage::PointType            PointType;
  typedef typename TInputImage::SizeType             SizeType;
  typedef typename TInputImage::SizeValueType        SizeValueType;
  typedef typename TInputImage::SpacingType          SpacingType;
  typedef typename TInputImage::RegionType           RegionType;

  /** Typedefs for parameters */
  typedef std::pair<std::string, tensorflow::Tensor> DictElementType;
  typedef std::vector<std::string>                   StringList;
  typedef std::vector<SizeType>                      SizeListType;
  typedef std::vector<DictElementType>               DictType;
  typedef std::vector<tensorflow::DataType>          DataTypeListType;
  typedef std::vector<tensorflow::TensorShapeProto>  TensorShapeProtoList;
  typedef std::vector<tensorflow::Tensor>            TensorListType;

  /** Set and Get the Tensorflow session and graph */
  void SetGraph(tensorflow::GraphDef graph)      { m_Graph = graph;     }
  tensorflow::GraphDef GetGraph()                { return m_Graph ;     }
  void SetSession(tensorflow::Session * session) { m_Session = session; }
  tensorflow::Session * GetSession()             { return m_Session;    }

  /** Model parameters */
  void PushBackInputTensorBundle(std::string name, SizeType receptiveField, ImagePointerType image);
  void PushBackOuputTensorBundle(std::string name, SizeType expressionField);

  /** Input placeholders names */
  itkSetMacro(InputPlaceholders, StringList);
  itkGetMacro(InputPlaceholders, StringList);

  /** Receptive field */
  itkSetMacro(InputReceptiveFields, SizeListType);
  itkGetMacro(InputReceptiveFields, SizeListType);

  /** Output tensors names */
  itkSetMacro(OutputTensors, StringList);
  itkGetMacro(OutputTensors, StringList);

  /** Expression field */
  itkSetMacro(OutputExpressionFields, SizeListType);
  itkGetMacro(OutputExpressionFields, SizeListType);

  /** User placeholders */
  void SetUserPlaceholders(DictType dict) { m_UserPlaceholders = dict; }
  DictType GetUserPlaceholders()          { return m_UserPlaceholders; }

  /** Target nodes names */
  itkSetMacro(TargetNodesNames, StringList);
  itkGetMacro(TargetNodesNames, StringList);

  /** Read only methods */
  itkGetMacro(InputTensorsDataTypes, DataTypeListType);
  itkGetMacro(OutputTensorsDataTypes, DataTypeListType);
  itkGetMacro(InputTensorsShapes, TensorShapeProtoList);
  itkGetMacro(OutputTensorsShapes, TensorShapeProtoList);

  virtual void GenerateOutputInformation();

protected:
  TensorflowMultisourceModelBase();
  virtual ~TensorflowMultisourceModelBase() {};

  virtual std::stringstream GenerateDebugReport(DictType & inputs);

  virtual void RunSession(DictType & inputs, TensorListType & outputs);

private:
  TensorflowMultisourceModelBase(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  // Tensorflow graph and session
  tensorflow::GraphDef       m_Graph;                   // The TensorFlow graph
  tensorflow::Session *      m_Session;                 // The TensorFlow session

  // Model parameters
  StringList                 m_InputPlaceholders;       // Input placeholders names
  SizeListType               m_InputReceptiveFields;    // Input receptive fields
  StringList                 m_OutputTensors;           // Output tensors names
  SizeListType               m_OutputExpressionFields;  // Output expression fields
  DictType                   m_UserPlaceholders;        // User placeholders
  StringList                 m_TargetNodesNames;        // User nodes target

  // Internal, read-only
  DataTypeListType           m_InputTensorsDataTypes;   // Input tensors datatype
  DataTypeListType           m_OutputTensorsDataTypes;  // Output tensors datatype
  TensorShapeProtoList       m_InputTensorsShapes;      // Input tensors shapes
  TensorShapeProtoList       m_OutputTensorsShapes;     // Output tensors shapes

}; // end class


} // end namespace otb

#include "otbTensorflowMultisourceModelBase.hxx"

#endif
