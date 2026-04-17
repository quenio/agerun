---
license: mit
license_link: >-
  https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/LICENSE
language:
- en
pipeline_tag: text-generation
tags:
- nlp
- code
---

## Model Summary

This repo provides the GGUF format for the Phi-3-Mini-4K-Instruct. 
The Phi-3-Mini-4K-Instruct is a 3.8B parameters, lightweight, state-of-the-art open model trained with the Phi-3 datasets that includes both synthetic data and the filtered publicly available websites data with a focus on high-quality and reasoning dense properties.
The model belongs to the Phi-3 family with the Mini version in two variants [4K](https://huggingface.co/microsoft/Phi-3-mini-4k-instruct) and [128K](https://huggingface.co/microsoft/Phi-3-mini-128k-instruct) which is the context length (in tokens) it can support. 
The model has underwent a post-training process that incorporates both supervised fine-tuning and direct preference optimization to ensure precise instruction adherence and robust safety measures.
When assessed against benchmarks testing common sense, language understanding, math, code, long context and logical reasoning, Phi-3 Mini-4K-Instruct showcased a robust and state-of-the-art performance among models with less than 13 billion parameters.

Resources and Technical Documentation:

+ [Phi-3 Microsoft Blog](https://aka.ms/phi3blog-april)
+ [Phi-3 Technical Report](https://aka.ms/phi3-tech-report)
+ [Phi-3 on Azure AI Studio](https://aka.ms/phi3-azure-ai)
+ [Phi-3 on Hugging Face](https://aka.ms/phi3-hf)
+ Phi-3 ONNX: [4K](https://aka.ms/phi3-mini-4k-instruct-onnx) and [128K](https://aka.ms/phi3-mini-128k-instruct-onnx)

This repo provides GGUF files for the Phi-3 Mini-4K-Instruct model. 
| Name | Quant method | Bits | Size | Use case |
| ---- | ---- | ---- | ---- | ----- |
| [Phi-3-mini-4k-instruct-q4.gguf](https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/blob/main/Phi-3-mini-4k-instruct-q4.gguf) | Q4_K_M | 4 | 2.2 GB| medium, balanced quality - recommended |
| [Phi-3-mini-4k-instruct-fp16.gguf](https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/blob/main/Phi-3-mini-4k-instruct-fp16.gguf) | None | 16 | 7.2 GB | minimal quality loss |


## Intended Uses

**Primary use cases**

The model is intended for commercial and research use in English. The model provides uses for applications which require 
1) memory/compute constrained environments
2) latency bound scenarios
3) strong reasoning (especially math and logic)
4) long context

Our model is designed to accelerate research on language and multimodal models, for use as a building block for generative AI powered features. 

**Use case considerations**

Our models are not specifically designed or evaluated for all downstream purposes. Developers should consider common limitations of language models as they select use cases, and evaluate and mitigate for accuracy, safety, and fariness before using within a specific downstream use case, particularly for high risk scenarios. 
Developers  should be aware of and adhere to applicable laws or regulations (including privacy, trade compliance laws, etc.) that are relevant to their use case. 
Nothing contained in this Model Card should be interpreted as or deemed a restriction or modification to the license the model is released under.  


## Chat Format:

Given the nature of the training data, the Phi-3-Mini-4K-instruct model is best suited for prompts using the chat format as follows. 
You can provide the prompt as a question with a generic template as follow:
```markdown
<|user|>\nQuestion <|end|>\n<|assistant|>
```
For example:
```markdown
<|user|>
How to explain Internet for a medieval knight?<|end|>
<|assistant|>
```

where the model generates the text after "<|assistant|>" . In case of few-shots prompt, the prompt can be formatted as the following:

```markdown
<|user|>
I am going to Paris, what should I see?<|end|>
<|assistant|>
Paris, the capital of France, is known for its stunning architecture, art museums, historical landmarks, and romantic atmosphere. Here are some of the top attractions to see in Paris:\n\n1. The Eiffel Tower: The iconic Eiffel Tower is one of the most recognizable landmarks in the world and offers breathtaking views of the city.\n2. The Louvre Museum: The Louvre is one of the world's largest and most famous museums, housing an impressive collection of art and artifacts, including the Mona Lisa.\n3. Notre-Dame Cathedral: This beautiful cathedral is one of the most famous landmarks in Paris and is known for its Gothic architecture and stunning stained glass windows.\n\nThese are just a few of the many attractions that Paris has to offer. With so much to see and do, it's no wonder that Paris is one of the most popular tourist destinations in the world."<|end|>
<|user|>
What is so great about #1?<|end|>
<|assistant|>
```

## How to download GGUF files

1. **Install Hugging Face CLI:**

```
pip install huggingface-hub>=0.17.1
```

2. **Login to Hugging Face:**
```
huggingface-cli login
```

3. **Download the GGUF model:**
```
huggingface-cli download microsoft/Phi-3-mini-4k-instruct-gguf Phi-3-mini-4k-instruct-q4.gguf --local-dir . --local-dir-use-symlinks False
```

## How to use with Ollama

1. **Install Ollama:**

```
curl -fsSL https://ollama.com/install.sh | sh
```

2. **Run the *phi3* model:**

```
ollama run phi3
```

### Building from `Modelfile`

Assuming that you have already downloaded GGUF files, here is how you can use them with [Ollama](https://ollama.com/):

1. **Get the Modelfile:**

```
huggingface-cli download microsoft/Phi-3-mini-4k-instruct-gguf Modelfile_q4 --local-dir /path/to/your/local/dir
```

2. Build the Ollama Model:
Use the Ollama CLI to create your model with the following command:

```
ollama create phi3 -f Modelfile_q4
```

3. **Run the *phi3* model:** 

Now you can run the Phi-3-Mini-4k-Instruct model with Ollama using the following command:

```
ollama run phi3 "Your prompt here"
```

Replace "Your prompt here" with the actual prompt you want to use for generating responses from the model.

## How to use with Llamafile:

Assuming that you already have GGUF files downloaded. Here is how you can use the GGUF model with [Llamafile](https://github.com/Mozilla-Ocho/llamafile):

1. **Download Llamafile-0.7.3**
```
wget https://github.com/Mozilla-Ocho/llamafile/releases/download/0.7.3/llamafile-0.7.3
```
2. **Run the model with chat format prompt:**


```markdown
<|user|>\nHow to explain Internet for a medieval knight?\n<|end|>\n<|assistant|>
```


```
./llamafile-0.7.3 -ngl 9999 -m Phi-3-mini-4k-instruct-q4.gguf --temp 0.6 -p "<|user|>\nHow to explain Internet for a medieval knight?\n<|end|>\n<|assistant|>"
```

3. **Run with a chat interface:**

```
./llamafile-0.7.3 -ngl 9999 -m Phi-3-mini-4k-instruct-q4.gguf
```

Your browser should open automatically and display a chat interface. (If it doesn't, just open your browser and point it at http://localhost:8080)

## How to run on Python:

1. **Install llama-cpp-python:**

```
! CMAKE_ARGS="-DLLAMA_CUBLAS=on" pip install llama-cpp-python
```

2. **Run the model:**

```python
from llama_cpp import Llama


llm = Llama(
  model_path="./Phi-3-mini-4k-instruct-q4.gguf",  # path to GGUF file
  n_ctx=4096,  # The max sequence length to use - note that longer sequence lengths require much more resources
  n_threads=8, # The number of CPU threads to use, tailor to your system and the resulting performance
  n_gpu_layers=35, # The number of layers to offload to GPU, if you have GPU acceleration available. Set to 0 if no GPU acceleration is available on your system.
)

prompt = "How to explain Internet to a medieval knight?"

# Simple inference example
output = llm(
  f"<|user|>\n{prompt}<|end|>\n<|assistant|>",
  max_tokens=256,  # Generate up to 256 tokens
  stop=["<|end|>"], 
  echo=True,  # Whether to echo the prompt
)

print(output['choices'][0]['text'])
```

## Responsible AI Considerations
Like other language models, the Phi series models can potentially behave in ways that are unfair, unreliable, or offensive. Some of the limiting behaviors to be aware of include:  
+ Quality of Service: the Phi models are trained primarily on English text. Languages other than English will experience worse performance. English language varieties with less representation in the training data might experience worse performance than standard American English.   
+ Representation of Harms & Perpetuation of Stereotypes: These models can over- or under-represent groups of people, erase representation of some groups, or reinforce demeaning or negative stereotypes. Despite safety post-training, these limitations may still be present due to differing levels of representation of different groups or prevalence of examples of negative stereotypes in training data that reflect real-world patterns and societal biases. 
+ Inappropriate or Offensive Content: these models may produce other types of inappropriate or offensive content, which may make it inappropriate to deploy for sensitive contexts without additional mitigations that are specific to the use case. 
+ Information Reliability: Language models can generate nonsensical content or fabricate content that might sound reasonable but is inaccurate or outdated.  
+ Limited Scope for Code: Majority of Phi-3 training data is based in Python and use common packages such as "typing, math, random, collections, datetime, itertools". If the model generates Python scripts that utilize other packages or scripts in other languages, we strongly recommend users manually verify all API uses.   

Developers should apply responsible AI best practices and are responsible for ensuring that a specific use case complies with relevant laws and regulations (e.g. privacy, trade, etc.). Important areas for consideration include: 
+ Allocation: Models may not be suitable for scenarios that could have consequential impact on legal status or the allocation of resources or life opportunities (ex: housing, employment, credit, etc.) without further assessments and additional debiasing techniques.
+ High-Risk Scenarios: Developers should assess suitability of using models in high-risk scenarios where unfair, unreliable or offensive outputs might be extremely costly or lead to harm. This includes providing advice in sensitive or expert domains where accuracy and reliability are critical (ex: legal or health advice). Additional safeguards should be implemented at the application level according to the deployment context. 
+ Misinformation: Models may produce inaccurate information. Developers should follow transparency best practices and inform end-users they are interacting with an AI system. At the application level, developers can build feedback mechanisms and pipelines to ground responses in use-case specific, contextual information, a technique known as Retrieval Augmented Generation (RAG).   
+ Generation of Harmful Content: Developers should assess outputs for their context and use available safety classifiers or custom solutions appropriate for their use case. 
+ Misuse: Other forms of misuse such as fraud, spam, or malware production may be possible, and developers should ensure that their applications do not violate applicable laws and regulations.


## Training

### Model

* Architecture: Phi-3 Mini has 3.8B parameters and is a dense decoder-only Transformer model. The model is fine-tuned with Supervised fine-tuning (SFT) and Direct Preference Optimization (DPO) to ensure alignment with human preferences and safety guidlines.
* Inputs: Text. It is best suited for prompts using chat format.
* Context length: 4K tokens
* GPUS: 512 H100-80G
* Training time: 7 days
* Training data: 3.3T tokens
* Outputs: Generated text in response to the input
* Dates: Our models were trained between February and April 2024
* Status: This is a static model trained on an offline dataset with cutoff date October 2023. Future versions of the tuned models may be released as we improve models.

### Datasets
Our training data includes a wide variety of sources, totaling 3.3 trillion tokens, and is a combination of 
1) publicly available documents filtered rigorously for quality, selected high-quality educational data, and code; 
2) newly created synthetic, “textbook-like” data for the purpose of teaching math, coding, common sense reasoning, general knowledge of the world (science, daily activities, theory of mind, etc.); 
3) high quality chat format supervised data covering various topics to reflect human preferences on different aspects such as instruct-following, truthfulness, honesty and helpfulness.

### Software

* [PyTorch](https://github.com/pytorch/pytorch)
* [DeepSpeed](https://github.com/microsoft/DeepSpeed)
* [Transformers](https://github.com/huggingface/transformers)
* [Flash-Attention](https://github.com/HazyResearch/flash-attention)

### License

The model is licensed under the [MIT license](https://huggingface.co/microsoft/phi-3-mini-128k/resolve/main/LICENSE).

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow [Microsoft’s Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks). Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those third-party’s policies.

## Data Summary
https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/blob/main/data_summary_card.md