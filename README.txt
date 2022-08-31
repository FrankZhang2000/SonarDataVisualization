水下声图像分类识别算法研究 实验源代码

1. 文件构成
    src/：存放全部源代码
    font/：存放可视化程序使用的矢量字体文件及对应的位图
    data/：存放原始数据
    saved_data/：存放可视化程序导出的数据和数据生成器生成的数据
    groupmetrics/：存放全部物体的配置文件、物体的各项特征和聚类程序
    build/：存放编译结果

2. 源代码文件的具体说明
    visualization.cpp：可视化程序的主文件
    datagenerator.cpp：数据生成器的主文件
    getmetrics.cpp：物体特征提取程序的主文件
    genbitmap.c：字体位图生成器的主文件
    classifier.h、classifier.cpp：包含物体检测、计算物体特征的代码
    joinobject.h、joinobject.cpp：包含数据生成器进行随机变换、噪声生成和带回溯搜索的代码
    objwriter.h、objwriter.cpp：包含导出声图数据的代码
    preprocessor.h、preprocessor.cpp：包含读取声图数据、分辨率增强、噪声检测及去除的代码
    fontformatter.h、fontformatter.cpp：包含文字渲染的代码
    settings.h：所需全部超参数的配置文件
    utils.h：定义了所需的简单数据结构
    charset.h：所需全部字符的配置文件，用于字体位图的生成

3. 依赖
    可视化程序（build/visualization）：OpenGL-4.1、matlab C++库
    数据生成器（build/datagenerator）：matlab C++库
    物体特征提取程序（build/getmetrics）：matlab C++库
    字体位图生成器（build/genbitmap）：FreeType-2.12.0
    聚类程序（cluster.py）：numpy、skLearn、pandas、matplotlib

4. 编译方法
    Step #1：修改Makefile中各个依赖库的路径
    Step #2：执行make
    清除编译结果：执行make clean

5. 使用方法
    可视化程序（build/visualization）：
        若需查看有关命令行参数的帮助信息，可执行./build/visualization -h
        若需查看有关用户输入的帮助信息，可在程序运行过程中输入h
        示例：
            对<file_name>文件进行可视化，使用默认工作参数：
                ./build/visualization --mat <file_name>
            对<file_name>文件进行可视化，将反射强度阈值设为32，并将物体检测尺度设为15：
                ./build/visualization --mat <file_name> --thre 32 --dist 15
            对<file_name>文件进行可视化，显示全部噪声点，并关闭分辨率增强功能：
                ./build/visualization --mat <file_name> --shownoise --noenhance
            对<file_name>文件进行可视化，将物体导出路径设为<save_dir>：
                ./build/visualization --mat <file_name> --savedir<save_dir>
    数据生成器（build/datagenerator）：
        若需查看有关命令行参数的帮助信息，可执行./build/datagenerator -h
        示例：
            使用两个固定物体<fixed_obj_1>、<fixed_obj_2>和两个移动物体<norm_obj_1>、<norm_obj_2>，生成<num>组数据，并以<file_name>为文件名前缀导出到<out_dir>路径下：
                ./build/datagenerator --fixed fixed_obj_1 --fixed fixed_obj_2 --norm norm_obj_1 --norm norm_obj_2 --outdir <out_dir> --name <file_name> --num <num>
            使用<config_file>文件作为物体配置文件，生成<num>组数据，并以<file_name>为文件名前缀导出到<out_dir>路径下：
                ./build/datagenerator --config <config_file> --outdir <out_dir> --name <file_name> --num <num>
                物体配置文件的格式：每一行表示一个物体，依次记录物体对应的声图数据路径、物体类型和物体数量，并使用空格隔开
    物体特征提取程序（build/getmetrics）：
        若需查看有关命令行参数的帮助信息，可执行./build/getmetrics -h
        示例：
            使用<config_file>文件作为物体配置文件，将各个物体的特征存放到名为<file_name>的csv文件：
                ./build/getmetrics --config <config_file> --out <file_name>
                物体配置文件的格式：每一行表示一个声图数据中的单个或多个物体，依次记录声图数据路径、反射强度阈值、物体检测尺度、离群点检测阈值、噪声检测阈值和各个物体的序号（以-1结尾），并使用空格隔开
    字体位图生成器（build/genbitmap）：
        若需查看有关命令行参数的帮助信息，可执行./build/genbitmap -h
        示例：
            使用<font_file>文件作为矢量字体文件，将字体位图存放在<file_name>文件中：
                ./build/genbitmap --font <font_file> --out <file_name>
    聚类程序（cluster.py）：
        示例：
            对名为<file_name>的csv文件中的各个物体进行聚类：
                python3 cluster.py <file_name>
